import sys

sys.path.append('..')

import RumiCar

import re
import gi
import asyncio
import websockets
import time
import json
import threading

gi.require_version('Gst', '1.0')

from gi.repository import Gst, GObject, GLib
from sys import getsizeof

Gst.init(None)

Pipeline = 'v4l2src device=/dev/video0 ! video/x-raw, width=640, height=480, framerate=30/1, format=NV12 ! v4l2h264enc extra-controls="controls, video_bitrate=500000, h264_profile=1, h264_level=9, h264_i_frame_period=60" ! video/x-h264, level=(string)3.1, profile=(string)baseline ! h264parse config-interval=1 ! queue max-size-buffers=0 max-size-time=0 max-size-bytes=0 leaky=downstream ! appsink max-buffers=1 name=moth drop=true sync=false emit-signals=true'
pipeline = Gst.parse_launch(Pipeline)

sink = pipeline.get_by_name("moth")

pipeline.set_state(Gst.State.PLAYING)

json_pattern = r'\{.*?\}'
mime_pattern = r'(\w+)=([\d.]+)'


def get_parse():
    sample = sink.emit("pull-sample")

    if sample is not None:
        current_buffer = sample.get_buffer()
        image = current_buffer.extract_dup(0, current_buffer.get_size())
        # print(f"{type(image)} {getsizeof(image)}")  # getsizeof returns bytes of image
        return image
    else:
        return False


async def Connect_and_Control():
    global velocity
    while True:
        try:
            async with websockets.connect(
                    "ws://cobot.center:8286/pang/ws/pub?channel=instant&name=RumiCar&track=video&mode=bundle",
                    ping_timeout=None) as ws:
                print("[Client_Video]Connected.")

                current_time = time.time()

                def data_received(data):
                    Message = data.decode("utf-8", "ignore")
                    # print(Message)

                    try:
                        json_str = None
                        mime_str = None
                        data = ''.join(Message.split())
                        mime_str = re.findall(mime_pattern, data)
                        json_str = re.findall(json_pattern, data)
                        if mime_str:
                            pass
                        if json_str:
                            for i in json_str:
                                json_data = json.loads(i)
                                for key, val in json_data.items():
                                    if key == "direction":
                                        if val == "N":
                                            RumiCar.rc_drive(RumiCar.FORWARD, velocity)
                                            RumiCar.rc_steer(RumiCar.CENTER)
                                        elif val == "S":
                                            RumiCar.rc_drive(RumiCar.REVERSE, velocity)
                                            RumiCar.rc_steer(RumiCar.CENTER)
                                        elif val == "CCW":
                                            RumiCar.rc_steer(RumiCar.LEFT)
                                        elif val == "CW":
                                            RumiCar.rc_steer(RumiCar.RIGHT)
                                        elif val == "NCW":
                                            RumiCar.rc_drive(RumiCar.FORWARD, velocity)
                                            RumiCar.rc_steer(RumiCar.RIGHT)
                                        elif val == "NCCW":
                                            RumiCar.rc_drive(RumiCar.FORWARD, velocity)
                                            RumiCar.rc_steer(RumiCar.LEFT)
                                        else:
                                            RumiCar.rc_drive(RumiCar.BRAKE, 0)
                                            RumiCar.rc_steer(RumiCar.CENTER)
                    except json.JSONDecodeError as e:
                        print(f"JSON Error: {e}")
                    except Exception as e:
                        print(f"Unknown Error: {e}")

                ws.data_received = data_received

                while True:

                    image_bytes = get_parse()
                    if image_bytes:
                        await ws.send(image_bytes)
                        await asyncio.sleep(0.0001)
                    if (time.time() - current_time > 1):
                        current_time = time.time()
                        await ws.send(
                            "video/h264;width=640;height=480;framerate=25;interval=10;bitrate=3000000;packet=codec;profile=high;")

        except Exception as e:
            print(f"[Exception]Error: {e}")
            if (ws.closed):
                print("[Client_Video]Trying to reconnect...")
                await asyncio.sleep(1)
                velocity = 800000
                await Connect_and_Control()
                print("[Client_Video]Reconnected.")


velocity = input("Enter velocity(0 ~ 900000): ")
if velocity == "":
    velocity = 800000
else:
    velocity = int(velocity)
asyncio.get_event_loop().run_until_complete(Connect_and_Control())
