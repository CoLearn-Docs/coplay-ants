import json
import subprocess
import asyncio
# from uart import Uart
from moth import Moth
from urllib.request import urlopen
import urllib

# Try WiFi connect
def connectWiFi(ssid, password):
	# Try WiFi connect by nmcli
	if(password == None):
		result = subprocess.run(['nmcli', 'dev', 'wifi', 'connect', ssid], capture_output=True)
	else:
		result = subprocess.run(['nmcli', 'dev', 'wifi', 'connect', ssid, 'password', password], capture_output=True)
	print(f"wifi connect result:{result}")
	# Wrong WiFi SSID
	if(result.stderr.decode("ascii").find("No network with SSID") > 0):
		print("wifi ssid failed")
	# Wrong WiFi Password
	elif(result.stderr.decode("ascii").find("security.psk") > 0):
		print("wifi password failed")
	else:
		print("wifi connect successfuly")

def startMoth():
	ssid = "TeamGRIT"
	password = "teamgrit8266"
	# host = "moth.coplay.kr"
	host = "cobot.center"
	# port = 8276
	port = 8286
	path = "pang/ws/pub?channel=instant&name=picarx&track=video&mode=bundle"
	profile = "RPI_CW_001"
	print(f"host:{host}, port:{port}, path:{path}, profile:{profile}")
	# Try WiFi Connect
	connectWiFi(ssid, password)
	url = f"ws://{host}:{port}/{path}"
	moth.url = url
	# Try Moth Connect
	moth.start()


def main():
	global moth
	try:
		# Create Moth Instance
		moth = Moth()
		# Start Publishing Video To Moth
		startMoth()

	except KeyboardInterrupt:
		print(f"KeyboardInterrupt")
		asyncio.run(moth.close())

if __name__ == '__main__':
    main()
