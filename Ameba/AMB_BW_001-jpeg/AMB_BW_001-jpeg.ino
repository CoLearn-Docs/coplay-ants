/*
Create by Kim Dongjin. 2023.12.20
 */

#include "WiFi.h"
#include "VideoStream.h"
#include <ArduinoHttpClient.h>
#include "BLEDevice.h"
#include <ArduinoJson.h>

#define UART_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define STRING_BUF_SIZE 100

// Default preset configurations for each video channel:
// Channel 0 : 1920 x 1080 30FPS H264
// Channel 1 : 1280 x 720  30FPS H264
// Channel 2 : 1280 x 720  30FPS MJPEG
#define CHANNEL 2

VideoSetting config(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);

char ssid[] = "TeamGRIT_5G";   // your network SSID (name)
char pass[] = "teamgrit8266";  // your network password
int status = WL_IDLE_STATUS;

WiFiClient wifi;
// char serverAddress[] = "moth.coplay.kr";
// int port = 8276;
// WebSocketClient client = WebSocketClient(wifi, serverAddress, port);
// WebSocketClient *client;  // = WebSocketClient(wifi, "", 0);
void *client;
bool sentMime = false;
uint32_t img_addr = 0;
uint32_t img_len = 0;

BLEService UartService(UART_SERVICE_UUID);
BLECharacteristic Rx(CHARACTERISTIC_UUID_RX);
BLECharacteristic Tx(CHARACTERISTIC_UUID_TX);
BLEAdvertData advdata;
BLEAdvertData scndata;
bool notify = false;
int lineCount = 0;
String publishInfo = "";


void savePublishInfo(String info) {
  int lineCountDelimiterIndex = info.lastIndexOf('#');
  Serial.print("-----lineCountDelimiterIndex: ");
  Serial.println(lineCountDelimiterIndex);
  if (lineCountDelimiterIndex > 0) {
    publishInfo = "";
    publishInfo = info.substring(0, lineCountDelimiterIndex);
    Serial.print("-----publishInfo: ");
    Serial.println(publishInfo);
    int lineDelimiterIndex = info.lastIndexOf('$');
    Serial.print("-----lineDelimiterIndex:");
    Serial.println(lineDelimiterIndex);
    String countString = info.substring(lineCountDelimiterIndex + 1, lineDelimiterIndex);
    Serial.print("-----countString: ");
    Serial.println(countString);
    if (countString) {
      lineCount = countString.toInt();
      lineCount--;
    }
  } else {
    Serial.print("-----lineCount:");
    Serial.println(lineCount);
    if (lineCount > 0) {
      int lineDelimiterIndex = info.lastIndexOf('$');
      Serial.print("-----lineDelimiterIndex:");
      Serial.println(lineDelimiterIndex);
      if (lineDelimiterIndex > 0) {
        publishInfo += info.substring(0, lineDelimiterIndex);
        Serial.print("-----publishInfo: ");
        Serial.println(publishInfo);
      }
      if (lineCount == 1) {
        Serial.print("-----Saved publish info: ");
        Serial.println(publishInfo);
        if (publishInfo) {
          StaticJsonDocument<512> doc;
          deserializeJson(doc, publishInfo.c_str(), DeserializationOption::NestingLimit(30));
          String ssid = doc["data"]["server"]["ssid"].as<String>();
          String password = doc["data"]["server"]["password"].as<String>();
          String host = doc["data"]["server"]["host"].as<String>();
          int port = doc["data"]["server"]["port"].as<int>();
          String path = doc["data"]["server"]["path"].as<String>();
          Serial.print("-----host: ");
          Serial.println(host);
          Serial.print("-----port: ");
          Serial.println(port);
          Serial.print("-----path: ");
          Serial.println(path);
          if (host && port && path) {
            connectMediaServer(host, port, path);
          }
        }
      }
    }
    lineCount--;
  }
}

void readCB(BLECharacteristic* chr, uint8_t connID) {
  printf("Characteristic %s read by connection %d \n", chr->getUUID().str(), connID);
}

void writeCB(BLECharacteristic* chr, uint8_t connID) {
  printf("Characteristic %s write by connection %d :\n", chr->getUUID().str(), connID);
  if (chr->getDataLen() > 0) {
    String data = chr->readString();
    Serial.print("Received string: ");
    Serial.println(data.c_str());
    savePublishInfo(data);
  }
}

void notifCB(BLECharacteristic* chr, uint8_t connID, uint16_t cccd) {
  if (cccd & GATT_CLIENT_CHAR_CONFIG_NOTIFY) {
    printf("Notifications enabled on Characteristic %s for connection %d \n", chr->getUUID().str(), connID);
    notify = true;
  } else {
    printf("Notifications disabled on Characteristic %s for connection %d \n", chr->getUUID().str(), connID);
    notify = false;
  }
}

void beginBLE() {
  advdata.addFlags(GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED);
  advdata.addCompleteName("BBC AMEBA-ROBOT");
  scndata.addCompleteServices(BLEUUID(UART_SERVICE_UUID));

  Rx.setWriteProperty(true);
  Rx.setWritePermissions(GATT_PERM_WRITE);
  Rx.setWriteCallback(writeCB);
  Rx.setBufferLen(STRING_BUF_SIZE);

  Tx.setReadProperty(true);
  Tx.setReadPermissions(GATT_PERM_READ);
  Tx.setReadCallback(readCB);
  Tx.setNotifyProperty(true);
  Tx.setCCCDCallback(notifCB);
  Tx.setBufferLen(STRING_BUF_SIZE);

  UartService.addCharacteristic(Rx);
  UartService.addCharacteristic(Tx);

  BLE.init();
  BLE.configAdvert()->setAdvData(advdata);
  BLE.configAdvert()->setScanRspData(scndata);
  BLE.configServer(1);
  BLE.addService(UartService);

  BLE.beginPeripheral();
}

void connectMediaServer(String host, int port, String path) {
  WebSocketClient ws = WebSocketClient(wifi, host, port);
  client = &ws;
  char* apath = "/pang/ws/pub?channel=instant&name=ameba82&track=video&mode=bundle";
  ws.begin(apath);
  Serial.println("-----client begin");
  delay(1000);
}





void setup() {
  Serial.begin(115200);
  beginBLE();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(".");
    delay(1000);
  }
  Serial.println("Connected WiFi");
  Camera.configVideoChannel(CHANNEL, config);
  Camera.videoInit();
  Camera.channelBegin(CHANNEL);
  delay(1000);
  printInfo();
}




void loop() {
  WebSocketClient *ws = (WebSocketClient *)client;
  if (ws != NULL) {
    if (ws->connected()) {
      if (sentMime) {
        Camera.getImage(CHANNEL, &img_addr, &img_len);
        // Serial.print("-----image size:");
        // Serial.println(img_len);
        ws->beginMessage(TYPE_BINARY);
        // int sentByteSize = client.write((uint8_t *)img_addr, img_len);
        ws->write((uint8_t*)img_addr, img_len);
        ws->endMessage();
        // Serial.print("-----sent byte size:");
        // Serial.println(sentByteSize);
      } else {
        char* mime = "image/jpeg;codecs=jpeg;width=1280;height=720";
        ws->beginMessage(TYPE_TEXT);
        size_t sentMimeSize = ws->print(mime);
        ws->endMessage();
        if (sentMimeSize > 0) {
          sentMime = true;
          Serial.print("-----sentMime:");
          Serial.println(mime);
          Serial.print("-----sentMimeSize:");
          Serial.println(sentMimeSize);
        }
      }
    } else {
      // Serial.println("not connected websocket");
    }
  }
  delay(1);
}

void printInfo(void) {
  Serial.println("------------------------------");
  Serial.println("- Summary of Streaming -");
  Serial.println("------------------------------");
  Camera.printInfo();
  IPAddress ip = WiFi.localIP();
  Serial.print("-----ip:");
  Serial.println(ip);
}
