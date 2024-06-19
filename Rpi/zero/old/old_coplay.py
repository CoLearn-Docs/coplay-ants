import subprocess
import asyncio
import serial
import re
import json
import time
import websockets
import threading
import io
import gi
gi.require_version("Gst", "1.0")
from gi.repository import Gst

led = None
line = []
uart = None 

profile = None
ssid = None
password = None
host = None
port = None
channel = None
channel_name = None
websocket_url = None

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
	def control_led(self, val):
		message = val + '$'
		uart.write(message.encode())
		
class ControlRobot():
	def send_direction(self, direction):
		print(f'direction:{direction}')
		value = f"{direction}$"
		uart.write(value.encode())


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
				jsonObject = json.loads(msg)
				type = jsonObject.get("type")
				print(f"type:{type}")
				if(type == "control"):
					direction = jsonObject.get("direction")
					print(f"direction:{direction}")
					if(direction):
						self.ctl.send_direction(direction)
					else:
						print(f"empty direction")
				else:
					print(f"empty type")

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

def websocket_url_set(host, port, path):
	global websocket_url
	websocket_url = f"ws://{host}:{port}/{path}"

def wifi_connect(ssid, pwd):
	subprocess.run(['nmcli', 'dev', 'wifi', 'connect', ssid, 'password', pwd])

def completeRead(value):
	print("received value:" + value)
	# print(websocket_url)
	# moth = MothClient(websocket_url)
	# asyncio.run(start(moth))

def readSerial():
	global profile, ssid, password, host, port, path
	lineCount = 0
	index = 0
	msg = ""
	while (True):
		c = uart.readline()
		value = c.decode()
		value = value.rstrip()
		print(value)
		lineCountDelimiterIndex = value.find('#')
		if lineCountDelimiterIndex > 0:
			splits = value.split("#")
			value = splits[0]
			lineCount = int(splits[1])
		msg = ("%s%s") % (msg, value)
		if(lineCount - 1 > index):
			index = index + 1
		else:
			# break
			lineCount = 0
			index = 0
			uart.reset_input_buffer()
			print(msg)
			if(msg):
				try:
					jsonObject = json.loads(msg)
					type = jsonObject.get("type")
					if(type == "metric"):
						data = jsonObject.get("data")
						if(data):
							server = data.get("server")
							if(server):
								ssid = server.get("ssid")
								password = server.get("password")
								host = server.get("host")
								port = server.get("port")
								path = server.get("path")
								wifi_connect(ssid, password)
								websocket_url_set(host, port, path)
								moth = MothClient(websocket_url)
								asyncio.run(start(moth))
							else:
								print(f"empty server")
						else:
							print(f"empty data")
					else:
						print(f"empty type")

				except json.JSONDecodeError as e:
					print(f"JSON Error: {e}")
				except Exception as e:
					print(f"unknown Error: {e}")

def main():
	global uart, led
	uart = serial.Serial('/dev/ttyAMA0', 115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE)
	led = ControlLED()
	try:
		readSerial()
	except KeyboardInterrupt:
		led.control_led("led_off")
		uart.close()

async def start(mothclient):
	await mothclient.connect()
	led.control_led("led_pub")
	task1 = asyncio.create_task(mothclient.recv())
	task2 = asyncio.create_task(mothclient.send())
	await asyncio.gather(task1, task2)

if __name__ == '__main__':
    main()
