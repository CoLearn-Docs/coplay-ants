//
// Created by Kim Dongjin on 2023/12/19
//

#include "esp_camera.h"
#include "string"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <Arduino.h>
#include <iostream>
#include <functional>
#include <utility>
#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>

#define DIRECTION_KEY_VALUE_DIRECTION_STOP "STOP"
#define DIRECTION_KEY_VALUE_DIRECTION_FORWARD "N"
#define DIRECTION_KEY_VALUE_DIRECTION_BACKWARD "S"
#define DIRECTION_KEY_VALUE_DIRECTION_LEFT "W"
#define DIRECTION_KEY_VALUE_DIRECTION_RIGHT "E"
#define DIRECTION_KEY_VALUE_DIRECTION_CW "CW"
#define DIRECTION_KEY_VALUE_DIRECTION_CCW "CCW"
#define DIRECTION_KEY_VALUE_DIRECTION_NW "NW"
#define DIRECTION_KEY_VALUE_DIRECTION_NE "NE"
#define DIRECTION_KEY_VALUE_DIRECTION_SW "SW"
#define DIRECTION_KEY_VALUE_DIRECTION_SE "SE"
#define DIRECTION_KEY_VALUE_DIRECTION_CENTER "CENTER"
#define SEND_VIDEO_MIME_INTERVAL 1000
#define THING_PROFILE_ESP_HW_001 "ESP_HW_001"
#define WEBSOCKET_BUFFER_SIZE_BYTE (32 * 1024)
// CS102/CS202 V1.0 pin Assign
#define motA1 3
#define motA2 2
#define motB1 5
#define motB2 4
#define HOLD 33
#define KEY 35
#define BUZ 34
#define svrH 13
#define svrV 14

//Server Information
#define SSID "TeamGRIT"
#define PASSWORD "teamgrit8266"
// #define HOST "moth.coplay.kr"
#define HOST "cobot.center"
// #define PORT 8276
#define PORT 8286
#define PATH "/pang/ws/pub?channel=instant&name=esp32_tank&track=colink&mode=bundle"

#define DEBUG true
// #define DEBUG false

using namespace websockets;

WebsocketsClient client;
sensor_t *cam_sensor = nullptr;
Adafruit_NeoPixel pixels(1, 45, NEO_GRB + NEO_KHZ800);
Servo SVR1, SVR2;

void sendVideoMime();

//관련소스: https://cafe.naver.com/beardu/383

/*
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
  switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
      {
        if (DEBUG) {
          Serial.println("-----WebSocket connected");
        }
        break;
      }
    case WEBSOCKET_EVENT_DATA:
      {
        try {
          delay(50);
          const char *receivedData = (const char *)data->data_ptr;
          StaticJsonDocument<512> doc;
          deserializeJson(doc, receivedData, DeserializationOption::NestingLimit(30));
          String direction = doc["direction"].as<String>();
          if (DEBUG) {
            Serial.printf("-----receivedData: %s\n", receivedData);
            Serial.printf("-----receivedData size: %d\n", strlen(receivedData));
            Serial.printf("-----control direction: %s\n", direction.c_str());
          }
          sendControlData(direction.c_str());
        } catch (int e) {
          if (DEBUG) {
            Serial.printf("-----control exception: %d\n", e);
          }
        }
        break;
      }
    case WEBSOCKET_EVENT_DISCONNECTED:
      {
        if (DEBUG) {
          Serial.println("-----WebSocket disconnected");
        }
        break;
      }
  }
}

void onEventsCallback(WebsocketsEvent event, String data) {  // data == NULL, static_cast<int> event == 0.
  if (event == WebsocketsEvent::ConnectionOpened) {
    if (DEBUG) {
      Serial.println("Connnection Opened");
    }
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    if (DEBUG) {
      Serial.println("Connection Closed");
    }
  } else if (event == WebsocketsEvent::GotPing) {
    if (DEBUG) {
      Serial.println("Got a Ping!");
    }
  } else if (event == WebsocketsEvent::GotPong) {
    if (DEBUG) {
      Serial.println("Got a Pong!");
    }
  }
}
*/

void sendControlData(const char *direction) {
}

void connectMediaServer() {
  const char *ssid = SSID;
  const char *password = PASSWORD;
  const char *host = HOST;
  const uint16_t port = PORT;
  const char *path = PATH;
  const char *profile = THING_PROFILE_ESP_HW_001;
  if (!WiFi.isConnected()) {
    WiFi.begin(ssid, password);
    while (WiFiClass::status() != WL_CONNECTED) {
      delay(1000);
      if (DEBUG) {
        Serial.println((String) "-----RSSI : " + WiFi.RSSI() + " dB");
        Serial.print(".");
      }
    }
    if (DEBUG) {
      Serial.print("\n");
      Serial.println("-----WiFi connected");
      Serial.printf("-----new ssid, password: %s, %s\n", ssid, password);
      Serial.printf("-----new host, port: %s, %d\n", host, port);
      Serial.printf("-----new path, profile: %s, %s\n", path, profile);
    }
  }

  bool connected = client.connect(host, port, path);
  delay(1000);
  if (connected) {
    sendVideoMime();
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();
    if (DEBUG) {
      Serial.println("Connected!");
      client.send("Hello Server");
    }
  } else {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
    if (DEBUG) {
      Serial.println("Not Connected!");
    }
  }
  client.onMessage([&](WebsocketsMessage message) {
    if (DEBUG) {
      Serial.print("Got Message: ");
      Serial.println(message.data());
    }
  });
}

esp_err_t initCameraConfig() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 39;
  config.pin_d1 = 37;
  config.pin_d2 = 36;
  config.pin_d3 = 38;
  config.pin_d4 = 40;
  config.pin_d5 = 42;
  config.pin_d6 = 21;
  config.pin_d7 = 19;
  config.pin_xclk = 20;
  config.pin_pclk = 41;
  config.pin_vsync = 16;
  config.pin_href = 15;
  config.pin_sscb_sda = 10;
  config.pin_sscb_scl = 11;
  config.pin_pwdn = -1;
  config.pin_reset = 12;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;  //range: 0~63
  config.fb_count = 1;       //변경하면 WiFi 오류발생?(2023.12.20)

  config.fb_location = CAMERA_FB_IN_DRAM;
  return esp_camera_init(&config);
}

void sendVideoMime() {
  const char *mime = "image/jpeg;codecs=jpeg;width=640;height=480";
  bool result = client.send(mime);
  if (DEBUG) {
    if (result) {
      Serial.println("-----Success send mime");
    } else {
      Serial.println("-----Failed send mime");
    }
  }
}

void sendVideoFrame() {
  if (client.available()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      if (DEBUG) {
        Serial.println("-----Camera capture failed");
      }
      esp_camera_fb_return(fb);
      return;
    }
    client.send((char *)fb->buf);

    if (DEBUG) {
      // Serial.printf("Sent video buffer size: %d\n", fb->len);
    }
    esp_camera_fb_return(fb);
  }
}

void initCamera() {
  esp_err_t err = initCameraConfig();
  if (err != ESP_OK) {
    if (DEBUG) {
      Serial.printf("-----Camera init failed with error 0x%x", err);
    }
    return;
  }
}

void initServos() {
  SVR1.setPeriodHertz(50);
  SVR1.attach(svrH, 1000, 2000);
  SVR2.attach(svrV, 1000, 2000);
  // SVR1.write(90);
  // SVR2.write(90);
}

void setup() {
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initServos();
  initCamera();
  connectMediaServer();
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.show();
}

void loop() {
  sendVideoFrame();
  if (client.available()) {
    client.poll();
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();
  } else {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
  }
  delay(10);
}