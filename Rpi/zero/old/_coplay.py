import subprocess

import asyncio
import serial
import re
import json
import time
import websockets
import getmac
import gi
gi.require_version("Gst", "1.0")
from gi.repository import Gst

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
LOCAL_NAME =                   'CoPlay'
mainloop = None
led = None

profile = None
ssid = None
password = None
host = None
port = None
channel = None
channel_name = None
websocket_url = None

json_pattern = r'\{.*?\}'
mime_pattern = r'(w+)=([\d.]+)'

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
	global profile, ssid, password, host, port, channel, channel_name
	def __init__(self, bus, index, service):
		Characteristic.__init__(self, bus, index, UART_RX_CHARACTERISTIC_UUID, ['write'], service)

	def WriteValue(self, value, options):
		print('remote: {}'.format(bytearray(value).decode()))

		try:
			json_str = None
			data = bytearray(value).decode()
			json_str = re.findall(json_pattern, data)

			if json_str:
				for i in json_str:
					json_data = json.loads(i)
					for key, val in json_data.items():
						print(f"Key:{key}, Val:{val}")
						if key == "profile":
							profile = val
						elif key == "ssid":
							ssid = val
						elif key == "password":
							password = val
						elif key == "host":
							host = val
						elif key == "port":
							port = val
						elif key == "channel":
							channel = val
						elif key == "channel_name":
							channel_name = val

		except json.JSONDecodeError as e:
			print(f"JSON Error: {e}")
		except Exception as e:
			print(f"unknown Error: {e}")

		wifi_connect(ssid, password)
		websocket_url_set(host, port, channel, channel_name)
		mainloop.quit()

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
        local_name = LOCAL_NAME + "-[" + getmac.get_mac_address() + "]"
        print("Advertising BLE Name:" + local_name)
        self.add_local_name(local_name)
        self.include_tx_power = True

class GStreamerClient():
	def __init__(self):
		Gst.init(None)

		self.width = 640
		self.height = 480
		self.framerate = 30
		self.bitrate = 4000000
		self.h264_profile = 1
		self.h264_level = 9
		self.level = 3.1
		self.profile = "baseline"
		self.format = "NV12"
		self.framesize = {'QQVGA': [160, 120], 'QCIF': [176, 144], 'HQVGA': [240, 176], '240_240': [240, 240], 'QVGA': [320, 240], 'CIF': [400, 296], 'HVGA': [480, 320], 'VGA': [640, 480], 'SVGA': [800, 640], 'XGA': [1024, 768], 'HD': [1280, 720], 'SXGA': [1280, 1024], 'UXGA': [1600, 1200], 'FHD': [1920, 1080], 'PHD': [720, 1280], 'P3MP': [864, 1536], 'QXGA': [2048, 1536], 'QHD': [2560, 1440], 'WQXGA': [2560, 1600], 'PFHD': [1080, 1920], 'QSXGA': [2560, 1920]}

		self.mime = None
		self.pipeline_cmd = None
		self.sink = None
		self.pipeline = None

		self.pipeline_set()
		self.mime_set()
		self.play_gstreamer()

	def pipeline_set(self):
		self.pipeline_cmd = f'v4l2src device=/dev/video0 ! video/x-raw, width={self.width}, height={self.height}, framerate={self.framerate}/1, format={self.format} ! videoflip method=rotate-180 ! v4l2h264enc extra-controls="controls, video_bitrate={self.bitrate}, h264_profile={self.h264_profile}, h264_level={self.h264_level}" ! video/x-h264, level=(string){self.level}, profile=(string){self.profile} ! h264parse config-interval=1 ! queue ! appsink name=moth drop=true emit-signals=true sync=false'

	def mime_set(self):
		self.mime = f"video/h264;width={self.width};height={self.height};framerate={self.framerate};bitrate={self.bitrate}"

	def play_gstreamer(self):
		self.pipeline = Gst.parse_launch(self.pipeline_cmd)
		self.sink = self.pipeline.get_by_name("moth")
		self.pipeline.set_state(Gst.State.PLAYING)

	def change_pipeline(self, info):
		try:
			for row in self.framesize:
				if int(self.framesize[row][0]) == int(info['width']):
					if int(self.framesize[row][1]) == int(info['height']):
						self.width = int(info['width'])
						self.height = int(info['height'])
			if int(info['framerate']) > 15 and int(info['framerate']) < 31:
				self.framerate = int(info['framerate'])
			if int(info['bitrate']) > 24999 and int(info['bitrate']) < 25000001:
				self.bitrate = int(info['bitrate'])

			self.pipeline_set()
			self.mime_set()

			self.pipeline.set_state(Gst.State.NULL)
			self.play_gstreamer()

		except Exception as e:
			print(f"Exception: {e}")

	def get_parse(self):
		sample = self.sink.emit("pull_sample")
		if sample is not None:
			current_buffer = sample.get_buffer()
			image = current_buffer.extract_dup(0, current_buffer.get_size())

			return image
		else:
			return False
		
class ControlLED():
	def __init__(self):
		self.ser = serial.Serial('/dev/ttyAMA0', 115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE)

	def control_led(self, val):
		message = val + '$'
		self.ser.write(message.encode())
		
class ControlRobot():
	def __init__(self):
		self.ser = serial.Serial('/dev/ttyAMA0', 115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE)
		self.enum = ["FORWARD", "BACKWARD", "CC", "CW", "STOP", "FCC", "FCW", "BCC", "BCW", "L", "R", "FL", "FR", "BL", "BR"]
		self.now = self.enum[4]

	def change_now(self, num):
		self.now = self.enum[num]

	def control_direction(self, val):
			if val == "FORWARD":
#				print("FORWARD")
				if self.now == self.enum[4] or self.now == self.enum[1]:
					self.change_now(0)
					message = 'FW$'
					print('forward')
					self.ser.write(message.encode())
				elif self.now == self.enum[2] or self.now == self.enum[7]:
					self.change_now(5)
					message = 'FCC$'
					self.ser.write(message.encode())
				elif self.now == self.enum[3] or self.now == self.enum[8]:
					self.change_now(6)
					message = 'FCW$'
					self.ser.write(message.encode())
				elif self.now == self.enum[9] or self.now == self.enum[12]:
					self.change_now(11)
					message = 'FL$'
					self.ser.write(message.encode())
				elif self.now == self.enum[10] or self.now == self.enum[11]:
					self.change_now(12)
					message = 'FR$'
					self.ser.write(message.encode())
				else:
					pass

			elif val == "FORWARD_STOP":
				if self.now == self.enum[0]:
					self.change_now(4)
					message = 'ST$'
					self.ser.write(message.encode())
				elif self.now == self.enum[5]:
					self.change_now(2)
					message = 'CC$'
					self.ser.write(message.encode())
				elif self.now == self.enum[6]:
					self.change_now(3)
					message = 'CW$'
					self.ser.write(message.encode())
				elif self.now == self.enum[11]:
					self.change_now(9)
					message = 'L$'
					self.ser.write(message.encode())
				elif self.now == self.enum[12]:
					self.change_now(10)
					message = 'R$'
					self.ser.write(message.encode())
				else:
					pass

			elif val == "BACKWARD":
				if self.now == self.enum[4] or self.now == self.enum[0]:
					self.change_now(1)
					message = 'BW$'
					self.ser.write(message.encode())
				elif self.now == self.enum[2] or self.now == self.enum[5]:
					self.change_now(8)
					message = 'BCW$'
					self.ser.write(message.encode())
				elif self.now == self.enum[3] or self.now == self.enum[6]:
					self.change_now(7)
					message = 'BCC$'
					self.ser.write(message.encode())
				elif self.now == self.enum[9] or self.now == self.enum[14]:
					self.change_now(13)
					message = 'BL$'
					self.ser.write(message.encode())
				elif self.now == self.enum[10] or self.now == self.enum[13]:
					self.change_now(14)
					message = 'BR$'
					self.ser.write(message.encode())
				else:
					pass

			elif val == "BACKWARD_STOP":
				if self.now == self.enum[1]:
					self.change_now(4)
					message = 'ST$'
					self.ser.write(message.encode())
				elif self.now == self.enum[8]:
					self.change_now(2)
					message = 'CC$'
					self.ser.write(message.encode())
				elif self.now == self.enum[7]:
					self.change_now(3)
					message = 'CW$'
					self.ser.write(message.encode())
				elif self.now == self.enum[13]:
					self.change_now(9)
					message = 'L$'
					self.ser.write(message.encode())
				elif self.now == self.enum[14]:
					self.change_now(10)
					message = 'R$'
					self.ser.write(message.encode())
				else:
					pass

			elif val == "CC":
				if self.now == self.enum[4] or self.now == self.enum[3] or self.now == self.enum[9] or self.now == self.enum[10]:
					self.change_now(2)
					message = 'CC$'
					self.ser.write(message.encode())
				elif self.now == self.enum[0] or self.now == self.enum[6] or self.now == self.enum[11] or self.now == self.enum[12]:
					self.change_now(5)
					message = 'FCC$'
					self.ser.write(message.encode())
				elif self.now == self.enum[1] or self.now == self.enum[7] or self.now == self.enum[13] or self.now == self.enum[14]:
					self.change_now(8)
					message = 'BCW$'
					self.ser.write(message.encode())
				else:
					pass

			elif val == "CC_STOP":
				if self.now == self.enum[2]:
					self.change_now(4)
					message = 'ST$'
					self.ser.write(message.encode())
				elif self.now == self.enum[5]:
					self.change_now(0)
					message = 'FW$'
					self.ser.write(message.encode())
				elif self.now == self.enum[8]:
					self.change_now(1)
					message = 'BW$'
					self.ser.write(message.encode())
				else:
					pass

			elif val == "CW":
				if self.now == self.enum[4] or self.now == self.enum[2] or self.now == self.enum[9] or self.now == self.enum[10]:
					self.change_now(3)
					message = 'CW$'
					self.ser.write(message.encode())
				elif self.now == self.enum[0] or self.now == self.enum[5] or self.now == self.enum[11] or self.now == self.enum[12]:
					self.change_now(6)
					message = 'FCW$'
					self.ser.write(message.encode())
				elif self.now == self.enum[1] or self.now == self.enum[8] or self.now == self.enum[13] or self.now == self.enum[14]:
					self.change_now(7)
					message = 'BCC$'
					self.ser.write(message.encode())
				else:
					pass

			elif val == "CW_STOP":
				if self.now == self.enum[3]:
					self.change_now(4)
					message = 'ST$'
					self.ser.write(message.encode())
				elif self.now == self.enum[6]:
					self.change_now(0)
					message = 'FW$'
					self.ser.write(message.encode())
				elif self.now == self.enum[7]:
					self.change_now(1)
					message = 'BW$'
					self.ser.write(message.encode())
				else:
					pass

			elif val == "LEFT":
				if self.now == self.enum[4] or self.now == self.enum[10] or self.now == self.enum[2] or self.now == self.enum[3]:
					self.change_now(9)
					message = 'L$'
					self.ser.write(message.encode())
				elif self.now == self.enum[0] or self.now == self.enum[6] or self.now == self.enum[5] or self.now == self.enum[12]:
					self.change_now(11)
					message = 'FL$'
					self.ser.write(message.encode())
				elif self.now == self.enum[1] or self.now == self.enum[7] or self.now == self.enum[8] or self.now == self.enum[14]:
					self.change_now(13)
					message = 'BL$'
					self.ser.write(message.encode())
				else:
					pass

			elif val == "LEFT_STOP":
				if self.now == self.enum[9]:
					self.change_now(4)
					message = 'ST$'
					self.ser.write(message.encode())
				elif self.now == self.enum[11]:
					self.change_now(0)
					message = 'FW$'
					self.ser.write(message.encode())
				elif self.now == self.enum[13]:
					self.change_now(1)
					message = 'BW$'
					self.ser.write(message.encode())
				else:
					pass

			elif val == "RIGHT":
				if self.now == self.enum[4] or self.now == self.enum[2] or self.now == self.enum[9] or self.now == self.enum[3]:
					self.change_now(10)
					message = 'R$'
					self.ser.write(message.encode())
				elif self.now == self.enum[0] or self.now == self.enum[6] or self.now == self.enum[5] or self.now == self.enum[11]:
					self.change_now(12)
					message = 'FR$'
					self.ser.write(message.encode())
				elif self.now == self.enum[1] or self.now == self.enum[7] or self.now == self.enum[8] or self.now == self.enum[13]:
					self.change_now(14)
					message = 'BR$'
					self.ser.write(message.encode())
				else:
					pass

			elif val == "RIGHT_STOP":
				if self.now == self.enum[10]:
					self.change_now(4)
					message = 'ST$'
					self.ser.write(message.encode())
				elif self.now == self.enum[12]:
					self.change_now(0)
					message = 'FW$'
					self.ser.write(message.encode())
				elif self.now == self.enum[14]:
					self.change_now(1)
					message = 'BW$'
					self.ser.write(message.encode())
				else:
					pass

			else:
				self.change_now(4)
				message = 'ST$'
				self.ser.write(message.encode())

class MothClient():
	def __init__(self, url):
		self.url = url
		self.gst = GStreamerClient()
		self.ctl = ControlRobot()
		self.now = time.time()
		self.ws = None

	async def connect(self):
		self.ws = await websockets.connect(self.url, ping_timeout=None)
		print("moth connected!!")

	async def recv(self):
		while True:
			msg = await self.ws.recv()

			try:
				data = ''.join(msg.split())
				mime_str = re.findall(mime_pattern, data)
				json_str = re.findall(json_pattern, data)

				if mime_str:
					info = {}
					for key, val in mime_str:
						info[key] = val
					self.gst.change_pipeline(info)

				if json_str:
					for i in json_str:
						json_data = json.loads(i)
						for key, val in json_data.items():
							print(f"Key:{key}, val:{val}")
							if key == "direction":
								self.ctl.control_direction(val)

			except json.JSONDecodeError as e:
				print(f"JSON Error: {e}")
			except Exception as e:
				print(f"unknown Error: {e}")

	async def send(self):
		while True:
			image = self.gst.get_parse()
			if image:
				await self.ws.send(image)
				await asyncio.sleep(0.01)
				# print('1')
			if time.time() - self.now > 10:
				await self.ws.send(self.gst.mime)
				self.now = time.time()

	async def close_connection(self):
		await self.ws.close()

def websocket_url_set(host, port, channel, channel_name):
	global websocket_url
#	websocket_url = f"ws://{host}:{port}/pang/ws/pub?channel={channel}&name={channel_name}&track=video_01&mode=bundle"
	websocket_url = f"ws://{host}:8286/pang/ws/pub?channel={channel}&name={channel_name}&track=video_01&mode=bundle"

def wifi_connect(ssid, pwd):
	subprocess.run(['nmcli', 'dev', 'wifi', 'connect', ssid, 'password', pwd])

def find_adapter(bus):
    remote_om = dbus.Interface(bus.get_object(BLUEZ_SERVICE_NAME, '/'),
                               DBUS_OM_IFACE)
    objects = remote_om.GetManagedObjects()
    for o, props in objects.items():
        if LE_ADVERTISING_MANAGER_IFACE in props and GATT_MANAGER_IFACE in props:
            return o
        print('Skip adapter:', o)
    return None

def main():
    global mainloop
    global led
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
    
    led = ControlLED()
    led.control_led("led_ready")

    try:
        mainloop.run()
    except KeyboardInterrupt:
        led.control_led("led_off")
        adv.Release()

async def start(mothclient):
	await mothclient.connect()

	task1 = asyncio.create_task(mothclient.recv())
	task2 = asyncio.create_task(mothclient.send())

	await asyncio.gather(task1, task2)

	led.control_led("led_pub")


if __name__ == '__main__':
    main()
    print(websocket_url)
#    moth = MothClient("ws://cobot.center:8286/pang/ws/pub?channel=instant&name=open&track=video_01&mode=bundle")
    moth = MothClient(websocket_url)
    asyncio.run(start(moth))
