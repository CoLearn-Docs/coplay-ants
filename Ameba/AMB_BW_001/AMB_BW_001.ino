/*

 Example guide:
 https://www.amebaiot.com/en/amebapro2-amb82-mini-arduino-video-rtsp/

 For recommended setting to achieve better video quality, please refer to our Ameba FAQ: https://forum.amebaiot.com/t/ameba-faq/1220
 */

#include "WiFi.h"
#include "StreamIO.h"
#include "VideoStream.h"
#include "AudioStream.h"
#include "AudioEncoder.h"
#include "RTSP.h"
#include <ArduinoHttpClient.h>
#include "malloc.h"
#include "Stream.h"

//Example
// https://www.amebaiot.com/en/amebapro2-amb82-mini-arduino-peripherals-examples/

// #define CHANNEL 0
#define CHANNEL 1

// Default preset configurations for each video channel:
// Channel 0 : 1920 x 1080 30FPS H264
// Channel 1 : 1280 x 720  30FPS H264
// Channel 2 : 1280 x 720  30FPS MJPEG

// Default audio preset configurations:
// 0 :  8kHz Mono Analog Mic
// 1 : 16kHz Mono Analog Mic
// 2 :  8kHz Mono Digital PDM Mic
// 3 : 16kHz Mono Digital PDM Mic

VideoSetting configV(CHANNEL);
AudioSetting configA(0);
Audio audio;
AAC aac;
RTSP rtsp;
StreamIO audioStreamer(1, 1);  // 1 Input Audio -> 1 Output AAC
StreamIO avMixStreamer(2, 1);  // 2 Input Video + Audio -> 1 Output RTSP

char ssid[] = "TeamGRIT_5G";   // your network SSID (name)
char pass[] = "teamgrit8266";  // your network password
int status = WL_IDLE_STATUS;

WiFiClient wifi;
// char serverAddress[] = "moth.coplay.kr";
// char serverAddress[] = "socketsbay.com";
char serverAddress[] = "agilertc.com";
// int port = 8276;
// int port = 80;
int port = 8276;
WebSocketClient client = WebSocketClient(wifi, serverAddress, port);
MMFModule module;
bool sentMime = false;

void setup() {
  Serial.begin(115200);

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 2 seconds for connection:
    delay(2000);
  }

  // Configure camera video channel with video format information
  // Adjust the bitrate based on your WiFi network quality
  //configV.setBitrate(2 * 1024 * 1024);     // Recommend to use 2Mbps for RTSP streaming to prevent network congestion
  Camera.configVideoChannel(CHANNEL, configV);
  Camera.videoInit();

  // Configure audio peripheral for audio data output
  audio.configAudio(configA);
  audio.begin();
  // Configure AAC audio encoder
  aac.configAudio(configA);
  aac.begin();

  // Configure RTSP with identical video format information and enable audio streaming
  rtsp.configVideo(configV);
  rtsp.configAudio(configA, CODEC_AAC);
  rtsp.begin();

  // Configure StreamIO object to stream data from audio channel to AAC encoder
  audioStreamer.registerInput(audio);
  audioStreamer.registerOutput(aac);
  if (audioStreamer.begin() != 0) {
    Serial.println("StreamIO link start failed");
  }

  // Configure StreamIO object to stream data from video channel and AAC encoder to rtsp output
  // avMixStreamer.registerInput1(Camera.getStream(CHANNEL));
  module = Camera.getStream(CHANNEL);
  // module.setSnapshotCallback(CHANNEL);
  avMixStreamer.registerInput1(module);
  avMixStreamer.registerInput2(aac);
  avMixStreamer.registerOutput(rtsp);
  if (avMixStreamer.begin() != 0) {
    Serial.println("StreamIO link start failed");
  }

  // Start data stream from video channel
  Camera.channelBegin(CHANNEL);

  delay(1000);
  printInfo();

  char *path = "/pang/ws/pub?channel=instant&name=ameba82&track=video&mode=bundle";
  // char* path = "/ws/v2/1/demo/";
  // char* path = "/pang/ws/echo?channel=instant&name=ameba82&track=video&mode=bundle";
  client.begin(path);
}

void loop() {
  // Do nothing
  if (client.connected()) {

    /*
    Serial.println("connected ws");
    MMFModule module = Camera.getStream(CHANNEL);
    client.beginMessage(TYPE_BINARY);
    // uint8_t *data = *module._p_mmf_context->priv;
    // Serial.print("data type:");
    // Serial.println(*module._p_mmf_context->priv);
    // void* buf = *module._p_mmf_context->priv;
    // sprintf(buf, "%u", *module._p_mmf_context->priv);    
    printf("%d\n", module._p_mmf_context->curr_queue);
    // printf("%p\n", *module._p_mmf_context->priv);
    // int sentByteSize = client.write((uint8_t *)data, strlen((char *)data));
    int sentByteSize = client.write((uint8_t *)module._p_mmf_context->priv, strlen((char *)module._p_mmf_context->priv));
    client.endMessage();
    Serial.print("sent byte size:");
    Serial.println(sentByteSize);
    MMFModule stream = Camera.getStream(CHANNEL);
    int group_role = stream._p_mmf_context->group_role;
    uint32_t state = stream._p_mmf_context->state;
    int32_t queue_num = stream._p_mmf_context->queue_num;
    int32_t curr_queue = stream._p_mmf_context->curr_queue;
    Serial.print("group role:");
    Serial.println(group_role);
    Serial.print("state:");
    Serial.println(state);
    Serial.print("queue num:");
    Serial.println(queue_num);
    Serial.print("current queue:");
    Serial.println(curr_queue);
    */

    if (sentMime) {
      Stream *vb = (Stream *)module._p_mmf_context->priv;

      // client.beginMessage(TYPE_BINARY);
      // int sentByteSize = client.write(vb, strlen((const char *)vb));
      // int sentByteSize = client.print(vb);
      // int sentByteSize = client.write(vb);
      // client.endMessage();
      // Serial.print("sentByteSize:");
      // Serial.println(sentByteSize);
      Serial.print("vb size:");
      Serial.println(vb->parseInt());
    } else {
      char *mime = "video/h264;codecs=avc1.42E03C;width=1280;height=720";
      client.beginMessage(TYPE_TEXT);
      size_t sentMimeSize = client.print(mime);
      client.endMessage();
      if (sentMimeSize > 0) {
        sentMime = true;
        Serial.print("-----sentMime:");
        Serial.println(mime);
        Serial.print("-----sentMimeSize:");
        Serial.println(sentMimeSize);
      }
    }
  } else {
    Serial.println("not connected ws");
  }
  delay(100);
  // delay(1000);
}

void printInfo(void) {
  Serial.println("------------------------------");
  Serial.println("- Summary of Streaming -");
  Serial.println("------------------------------");
  Camera.printInfo();

  IPAddress ip = WiFi.localIP();

  Serial.println("- RTSP -");
  Serial.print("rtsp://");
  Serial.print(ip);
  Serial.print(":");
  rtsp.printInfo();

  Serial.println("- Audio -");
  audio.printInfo();
}

void createWebSocket() {
}
