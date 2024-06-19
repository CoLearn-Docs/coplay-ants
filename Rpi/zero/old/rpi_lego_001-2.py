import serial
import re
import json
import time
import asyncio
import websockets
from sys import getsizeof
import gi
gi.require_version("Gst", "1.0")
from gi.repository import Gst
import threading


# =========================================================================================
'''
BLE 관련 코드 시작
'''
# =========================================================================================
import sys
import dbus, dbus.mainloop.glib
from gi.repository import GLib
from example_advertisement import Advertisement
from example_advertisement import register_ad_cb, register_ad_error_cb
from example_gatt_server import Service, Characteristic
from example_gatt_server import register_app_cb, register_app_error_cb

BLUEZ_SERVICE_NAME =           'org.bluez'
DBUS_OM_IFACE =                'org.freedesktop.DBus.ObjectManager'
LE_ADVERTISING_MANAGER_IFACE = 'org.bluez.LEAdvertisingManager1'
GATT_MANAGER_IFACE =           'org.bluez.GattManager1'
GATT_CHRC_IFACE =              'org.bluez.GattCharacteristic1'
UART_SERVICE_UUID =            '6e400001-b5a3-f393-e0a9-e50e24dcca9e'
UART_RX_CHARACTERISTIC_UUID =  '6e400002-b5a3-f393-e0a9-e50e24dcca9e'
UART_TX_CHARACTERISTIC_UUID =  '6e400003-b5a3-f393-e0a9-e50e24dcca9e'
LOCAL_NAME =                   'CoPlay1234'
mainloop = None

class TxCharacteristic(Characteristic):
    def __init__(self, bus, index, service):
        Characteristic.__init__(self, bus, index, UART_TX_CHARACTERISTIC_UUID,
                                ['notify'], service)
        self.notifying = False
        GLib.io_add_watch(sys.stdin, GLib.IO_IN, self.on_console_input)

    def on_console_input(self, fd, condition):
        s = fd.readline()
        if s.isspace():
            pass
        else:
            self.send_tx(s)
        return True

    def send_tx(self, s):
        if not self.notifying:
            return
        value = []
        for c in s:
            value.append(dbus.Byte(c.encode()))
        self.PropertiesChanged(GATT_CHRC_IFACE, {'Value': value}, [])

    def StartNotify(self):
        if self.notifying:
            return
        self.notifying = True

    def StopNotify(self):
        if not self.notifying:
            return
        self.notifying = False

class RxCharacteristic(Characteristic):
    def __init__(self, bus, index, service):
        Characteristic.__init__(self, bus, index, UART_RX_CHARACTERISTIC_UUID,
                                ['write'], service)

    def WriteValue(self, value, options):
        print('remote: {}'.format(bytearray(value).decode()))

class UartService(Service):
    def __init__(self, bus, index):
        Service.__init__(self, bus, index, UART_SERVICE_UUID, True)
        self.add_characteristic(TxCharacteristic(bus, 0, self))
        self.add_characteristic(RxCharacteristic(bus, 1, self))

class Application(dbus.service.Object):
    def __init__(self, bus):
        self.path = '/'
        self.services = []
        dbus.service.Object.__init__(self, bus, self.path)

    def get_path(self):
        return dbus.ObjectPath(self.path)

    def add_service(self, service):
        self.services.append(service)

    @dbus.service.method(DBUS_OM_IFACE, out_signature='a{oa{sa{sv}}}')
    def GetManagedObjects(self):
        response = {}
        for service in self.services:
            response[service.get_path()] = service.get_properties()
            chrcs = service.get_characteristics()
            for chrc in chrcs:
                response[chrc.get_path()] = chrc.get_properties()
        return response

class UartApplication(Application):
    def __init__(self, bus):
        Application.__init__(self, bus)
        self.add_service(UartService(bus, 0))
		
class UartAdvertisement(Advertisement):
    def __init__(self, bus, index):
        Advertisement.__init__(self, bus, index, 'peripheral')
        self.add_service_uuid(UART_SERVICE_UUID)
        self.add_local_name(LOCAL_NAME)
        self.include_tx_power = True

def find_adapter(bus):
    remote_om = dbus.Interface(bus.get_object(BLUEZ_SERVICE_NAME, '/'),
                               DBUS_OM_IFACE)
    objects = remote_om.GetManagedObjects()
    for o, props in objects.items():
        if LE_ADVERTISING_MANAGER_IFACE in props and GATT_MANAGER_IFACE in props:
            return o
        print('Skip adapter:', o)
    return None

def beginBLE():
    global mainloop
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SystemBus()
    adapter = find_adapter(bus)
    if not adapter:
        print('BLE adapter not found')
        return

    service_manager = dbus.Interface(
                                bus.get_object(BLUEZ_SERVICE_NAME, adapter),
                                GATT_MANAGER_IFACE)
    ad_manager = dbus.Interface(bus.get_object(BLUEZ_SERVICE_NAME, adapter),
                                LE_ADVERTISING_MANAGER_IFACE)

    app = UartApplication(bus)
    adv = UartAdvertisement(bus, 0)

    mainloop = GLib.MainLoop()

    service_manager.RegisterApplication(app.get_path(), {},
                                        reply_handler=register_app_cb,
                                        error_handler=register_app_error_cb)
    ad_manager.RegisterAdvertisement(adv.get_path(), {},
                                     reply_handler=register_ad_cb,
                                     error_handler=register_ad_error_cb)
    try:
        mainloop.run()
    except KeyboardInterrupt:
        adv.Release()
        mainloop.quit()


# =========================================================================================
'''
BLE 관련 코드 종료
'''
# =========================================================================================



Gst.init(None)

ser = serial.Serial('/dev/ttyAMA0', 115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE)

framesize = {'QQVGA': [160, 120], 'QCIF': [176, 144], 'HQVGA': [240, 176], '240_240': [240, 240], 'QVGA': [320, 240], 'CIF': [400, 296], 'HVGA': [480, 320], 'VGA': [640, 480], 'SVGA': [800, 640], 'XGA': [1024, 768], 'HD': [1280, 720], 'SXGA': [1280, 1024], 'UXGA': [1600, 1200], 'FHD': [1920, 1080], 'PHD': [720, 1280], 'P3MP': [864, 1536], 'QXGA': [2048, 1536], 'QHD': [2560, 1440], 'WQXGA': [2560, 1600], 'PFHD': [1080, 1920], 'QSXGA': [2560, 1920]}

#websocket_address = "ws://cobot.center:8286/pang/ws/pub?channel=instant&name=raspberrypicar&track=video_01&mode=bundle"
websocket_address = "ws://cobot.center:8286/pang/ws/pub?channel=instant&name=rpicar&track=video_02&mode=bundle"

# [width, height, framerate, h264_level, level]
frame_720p_30 = [1280, 720, 30, 9, 3.1]
frame_480p_30 = [640, 480, 20, 9, 3.1]
bitrate = 2000000
h264_profile = 1	# BaseLine
frame = frame_480p_30

pipeline_cmd = ""
mime = ""

pipeline = None
sink = None

json_pattern = r'\{.*?\}'
mime_pattern = r'(\w+)=([\d.]+)'

enum = ["FORWARD", "BACKWARD", "CC", "CW", "STOP", "FCC", "FCW", "BCC", "BCW", "L", "R", "FL", "FR", "BL", "BR"]
global now
now = enum[4]

def pipeline_set():
	global pipeline_cmd
	pipeline_cmd = f'v4l2src device=/dev/video0 ! video/x-raw, width={frame[0]}, height={frame[1]}, framerate={frame[2]}/1, format=NV12 ! videoflip method=rotate-180 ! v4l2h264enc extra-controls="controls, video_bitrate={bitrate}, h264_profile={h264_profile}, h264_level={frame[3]}, h264_i_frame_period=60" min-force-key-unit-interval=60 ! video/x-h264, level=(string){frame[4]}, profile=(string)baseline ! h264parse config-interval=2 ! queue ! appsink name=moth drop=true emit-signals=true'
#	pipeline_cmd = f'libcamerasrc ! video/x-raw, width={frame[0]}, height={frame[1]}, framerate={frame[2]}/1, format=NV12 ! videoconvert ! v4l2h264enc extra-controls="controls, video_bitrate={bitrate}, h264_profile={h264_profile}, h264_level={frame[3]}, h264_i_frame_period=60" min-force-key-unit-interval=60 ! video/x-h264, level=(string){frame[4]}, profile=(string)baseline ! h264parse config-interval=2 ! queue ! appsink name=moth drop=true emit-signals=true'
	print(pipeline_cmd)

def play_gstreamer():
	global pipeline, sink
	pipeline = Gst.parse_launch(pipeline_cmd)
	sink = pipeline.get_by_name("moth")
	pipeline.set_state(Gst.State.PLAYING)

def change_pipeline(info):
	global frame, bitrate, pipeline, setup_pipeline, sink
	try:
		for row in framesize:
			if int(framesize[row][0]) == int(info['width']):
				if int(framesize[row][1]) == int(info['height']):
					frame[0] = int(info['width'])
					frame[1] = int(info['height'])
		if int(info['framerate']) > 15 and int(info['framerate']) < 31:
			frame[2] = int(info['framerate'])
		if int(info['bitrate']) > 24999 and int(info['bitrate']) < 25000001:
			bitrate = int(info['bitrate'])

		pipeline_set()
		mime_set()

		pipeline.set_state(Gst.State.NULL)
		play_gstreamer()

	except Exception as e:
		print(f"Exception: {e}")
		return

def mime_set():
	global mime
	mime = f"video/h264;width={frame[0]};height={frame[1]};framerate={frame[2]};bitrate={bitrate};"

def gst_parse():
    sample = sink.emit("pull_sample")
    if sample is not None:
        current_buffer = sample.get_buffer()
        image = current_buffer.extract_dup(0, current_buffer.get_size())
#        print(f"{type(image)}, {getsizeof(image)}")

        return image
    else:
        return False

def change_now(val):
	global now
#	print(enum[int(val)])
	now = enum[int(val)]

def control_direction(val):
#	global now
#	message = ""
	if val == "FORWARD":
		if now == enum[4] or now == enum[1]:
			change_now(0)
#			now = enum[0]
			message = 'FW$'
			ser.write(message.encode())
		elif now == enum[2] or now == enum[7]:
			change_now(5)
#			now = enum[5]
			message = 'FCC$'
			ser.write(message.encode())
		elif now == enum[3] or now == enum[8]:
			change_now(6)
#			now = enum[6]
			message = 'FCW$'
			ser.write(message.encode())
		elif now == enum[9] or now == enum[12]:
			change_now(11)
#			now = enum[6]
			message = 'FL$'
			ser.write(message.encode())
		elif now == enum[10] or now == enum[11]:
			change_now(12)
#			now = enum[6]
			message = 'FR$'
			ser.write(message.encode())
		else:
			pass

	elif val == "FORWARD_STOP":
		if now == enum[0]:
			change_now(4)
#			now = enum[4]
			message = 'ST$'
			ser.write(message.encode())
		elif now == enum[5]:
			change_now(2)
#			now = enum[2]
			message = 'CC$'
			ser.write(message.encode())
		elif now == enum[6]:
			change_now(3)
#			now = enum[3]
			message = 'CW$'
			ser.write(message.encode())
		elif now == enum[11]:
			change_now(9)
#			now = enum[3]
			message = 'L$'
			ser.write(message.encode())
		elif now == enum[12]:
			change_now(10)
#			now = enum[3]
			message = 'R$'
			ser.write(message.encode())
		else:
			pass

	elif val == "BACKWARD":
		if now == enum[4] or now == enum[0]:
			change_now(1)
#			now = enum[1]
			message = 'BW$'
			ser.write(message.encode())
		elif now == enum[2] or now == enum[5]:
			change_now(8)
#			now = enum[8]
			message = 'BCW$'
			ser.write(message.encode())
		elif now == enum[3] or now == enum[6]:
			change_now(7)
#			now = enum[7]
			message = 'BCC$'
			ser.write(message.encode())
		elif now == enum[9] or now == enum[14]:
			change_now(13)
#			now = enum[7]
			message = 'BL$'
			ser.write(message.encode())
		elif now == enum[10] or now == enum[13]:
			change_now(14)
#			now = enum[7]
			message = 'BR$'
			ser.write(message.encode())
		else:
			pass

	elif val == "BACKWARD_STOP":
		if now == enum[1]:
			change_now(4)
#			now = enum[4]
			message = 'ST$'
			ser.write(message.encode())
		elif now == enum[8]:
			change_now(2)
#			now = enum[2]
			message = 'CC$'
			ser.write(message.encode())
		elif now == enum[7]:
			change_now(3)
#			now = enum[3]
			message = 'CW$'
			ser.write(message.encode())
		elif now == enum[13]:
			change_now(9)
#			now = enum[3]
			message = 'L$'
			ser.write(message.encode())
		elif now == enum[14]:
			change_now(10)
#			now = enum[3]
			message = 'R$'
			ser.write(message.encode())
		else:
			pass

	elif val == "CC":
		if now == enum[4] or now == enum[3] or now == enum[9] or now == enum[10]:
			change_now(2)
#			now = enum[2]
			message = 'CC$'
			ser.write(message.encode())
		elif now == enum[0] or now == enum[6] or now == enum[11] or now == enum[12]:
			change_now(5)
#			now = enum[5]
			message = 'FCC$'
			ser.write(message.encode())
		elif now == enum[1] or now == enum[7] or now == enum[13] or now == enum[14]:
			change_now(8)
#			now = enum[8]
			message = 'BCW$'
			ser.write(message.encode())
		else:
			pass

	elif val == "CC_STOP":
		if now == enum[2]:
			change_now(4)
#			now = enum[4]
			message = 'ST$'
			ser.write(message.encode())
		elif now == enum[5]:
			change_now(0)
#			now = enum[0]
			message = 'FW$'
			ser.write(message.encode())
		elif now == enum[8]:
			change_now(1)
#			now = enum[1]
			message = 'BW$'
			ser.write(message.encode())
		else:
			pass

	elif val == "CW":
		if now == enum[4] or now == enum[2] or now == enum[9] or now == enum[10]:
			change_now(3)
#			now = enum[3]
			message = 'CW$'
			ser.write(message.encode())
		elif now == enum[0] or now == enum[5] or now == enum[11] or now == enum[12]:
			change_now(6)
#			now = enum[6]
			message = 'FCW$'
			ser.write(message.encode())
		elif now == enum[1] or now == enum[8] or now == enum[13] or now == enum[14]:
			change_now(7)
#			now = enum[7]
			message = 'BCC$'
			ser.write(message.encode())
		else:
			pass

	elif val == "CW_STOP":
		if now == enum[3]:
			change_now(4)
#			now = enum[4]
			message = 'ST$'
			ser.write(message.encode())
		elif now == enum[6]:
			change_now(0)
#			now = enum[0]
			message = 'FW$'
			ser.write(message.encode())
		elif now == enum[7]:
			change_now(1)
#			now = enum[1]
			message = 'BW$'
			ser.write(message.encode())
		else:
			pass

	elif val == "LEFT":
		if now == enum[4] or now == enum[10] or now == enum[2] or now == enum[3]:
			change_now(9)
#			now = enum[9]
			message = 'L$'
			ser.write(message.encode())
		elif now == enum[0] or now == enum[6] or now == enum[5] or now == enum[12]:
			change_now(11)
#			now = enum[5]
			message = 'FL$'
			ser.write(message.encode())
		elif now == enum[1] or now == enum[7] or now == enum[8] or now == enum[14]:
			change_now(13)
#			now = enum[8]
			message = 'BL$'
			ser.write(message.encode())
		else:
			pass

	elif val == "LEFT_STOP":
		if now == enum[9]:
			change_now(4)
#			now = enum[4]
			message = 'ST$'
			ser.write(message.encode())
		elif now == enum[11]:
			change_now(0)
#			now = enum[0]
			message = 'FW$'
			ser.write(message.encode())
		elif now == enum[13]:
			change_now(1)
#			now = enum[1]
			message = 'BW$'
			ser.write(message.encode())
		else:
			pass

	elif val == "RIGHT":
		if now == enum[4] or now == enum[2] or now == enum[9] or now == enum[3]:
			change_now(10)
#			now = enum[3]
			message = 'R$'
			ser.write(message.encode())
		elif now == enum[0] or now == enum[6] or now == enum[5] or now == enum[11]:
			change_now(12)
#			now = enum[5]
			message = 'FR$'
			ser.write(message.encode())
		elif now == enum[1] or now == enum[7] or now == enum[8] or now == enum[13]:
			change_now(14)
#			now = enum[8]
			message = 'BR$'
			ser.write(message.encode())
		else:
			pass

	elif val == "RIGHT_STOP":
		if now == enum[10]:
			change_now(4)
#			now = enum[4]
			message = 'ST$'
			ser.write(message.encode())
		elif now == enum[12]:
			change_now(0)
#			now = enum[0]
			message = 'FW$'
			ser.write(message.encode())
		elif now == enum[14]:
			change_now(1)
#			now = enum[1]
			message = 'BW$'
			ser.write(message.encode())
		else:
			pass

	else:
		change_now(4)
#		now = enum[4]
		message = 'ST$'
		ser.write(message.encode())
#	print(message)

async def connect():
	async with websockets.connect(websocket_address, ping_timeout=None) as ws1:
		print("moth connected")

		global msg
		def data_received(data):
			global msg
			msg = data.decode('utf-8', 'ignore')
#			print(msg)
			try:
				json_str = None
				mime_str = None
				data = ''.join(msg.split())
				mime_str = re.findall(mime_pattern, data)
				json_str = re.findall(json_pattern, data)
#				print(json_str[0])
				if mime_str:
					info = {}
					for key, val in mime_str:
						info[key] = val
					change_pipeline(info)
				if json_str:
#					json_str = json_str.group(0)
					for i in json_str:
						json_data = json.loads(i)
						for key, val in json_data.items():
							print(f"Key:{key}, val:{val}")
							if key == "direction":
								control_direction(val)
			except json.JSONDecodeError as e:
				print(f"JSON Error: {e}")
			except Exception as e:
				print(f"unknown Error: {e}")
		ws1.data_received = data_received

		now = time.time()
		while True:
#			try:
			image_bytes = gst_parse()
			if time.time() - now > 1:
				await ws1.send(mime)
				now = time.time()
			if image_bytes:
				await ws1.send(image_bytes)
				await asyncio.sleep(0.01)
#			except ws1.exceptions.ConnectionClosedError:
#				print("Connection closed. Reconnecting..")
#				break

# beginBLE()
# t = threading.Thread(target=beginBLE)
# t.start()


def beginPub():
	pipeline_set()
	mime_set()
	play_gstreamer()
	asyncio.get_event_loop().run_until_complete(connect())
	asyncio.get_event_loop().close()
	ser.close()

# t = threading.Thread(target=beginPub)
# t.start()

beginBLE()
