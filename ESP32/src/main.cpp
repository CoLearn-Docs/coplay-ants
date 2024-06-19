//
// Created by Park Juchan on 2023/03/31.
// Modified by Kim Dongjin on 2023/05/15.
//
#define CAMERA_MODEL_AI_THINKER

#include "CPBle.h"
#include "local_msg_generated.h"
#include "esp_camera.h"
#include "esp_wifi.h"
#include "esp_websocket_client.h"
#include "string"
#include "WiFi.h"
#include "camera_pins.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp32-hal-psram.h"
#include "lwip/opt.h"
#include <ArduinoJson.h>
#include "base64.hpp"
#include <ArduinoWebsockets.h>

#define DIRECTION_KEY_VALUE_DIRECTION_STOP "STOP"
#define DIRECTION_KEY_VALUE_DIRECTION_FORWARD "N"
#define DIRECTION_KEY_VALUE_DIRECTION_BACKWARD "S"
#define DIRECTION_KEY_VALUE_DIRECTION_LEFT "L"
#define DIRECTION_KEY_VALUE_DIRECTION_RIGHT "R"
#define DIRECTION_KEY_VALUE_DIRECTION_CW "CW"
#define DIRECTION_KEY_VALUE_DIRECTION_CCW "CCW"
#define DIRECTION_KEY_VALUE_DIRECTION_NW "nw"
#define DIRECTION_KEY_VALUE_DIRECTION_NE "ne"
#define DIRECTION_KEY_VALUE_DIRECTION_SW "sw"
#define DIRECTION_KEY_VALUE_DIRECTION_SE "se"
#define DIRECTION_KEY_VALUE_DIRECTION_CENTER "center"
#define BLE_NAME_PREFIX "CoPlay"
#define SEND_VIDEO_MIME_INTERVAL 1000
#define VIDEO_TRACK_NAME "camera"
#define THING_PROFILE_ESP_STANDARD_024 "ESP_STANDARD_024"
#define THING_PROFILE_ESP_ADVANCED_076 "ESP_ADVANCED_076"
#define THING_PROFILE_ESP_LEGO_001 "ESP_LEGO_001"

#define WEBSOCKET_BUFFER_SIZE_BYTE (32 * 1024)

// #define DEBUG true
#define DEBUG false

#define IN_1 2
#define IN_2 14
#define IN_3 15
#define IN_4 13

byte txdata[3] = {0xA5, 0, 0x5A};
const int Forward = 92;
const int Backward = 163;
const int Turn_Left = 149;
const int Turn_Right = 106;
const int Top_Left = 20;
const int Bottom_Left = 129;
const int Top_Right = 72;
const int Bottom_Right = 34;
const int Stop = 0;
const int Contrarotate = 172;
const int Clockwise = 83;
const int Moedl1 = 25;
const int Moedl2 = 26;
const int Moedl3 = 27;
const int Moedl4 = 28;
const int MotorLeft = 230;
const int MotorRight = 231;

CPBle ble = CPBle();
static esp_websocket_client_handle_t client;
sensor_t *cam_sensor = nullptr;
// unsigned long now = 0;
// unsigned long prevous = 0;
flatbuffers::FlatBufferBuilder builder(1024);
int prevSecond = -1;
int fps = 0;
int prevMinute = -1;
int fpm = 0;
String profile = THING_PROFILE_ESP_STANDARD_024;
bool flag_mime = true;
// String profile = THING_PROFILE_ESP_ADVANCED_076;
// String profile = THING_PROFILE_ESP_LEGO_001;

using namespace CoPlay::LocalMessage;

using namespace websockets;
WebsocketsClient client2;

void beginBLE();
String getMacAddress();
void connectGateway(const char *ssid, const char *password, const char *host, const uint16_t port, const char *path);

void controlLego001(const char *direction)
{
    if (DEBUG)
    {
        Serial.print("-----R001 direction:");
        Serial.print(direction);
    }
    Serial.write(direction);
    Serial.write('$');
}

void controlS024(const char *direction)
{
    if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_FORWARD) == 0)
    {
        // FORWARD
        digitalWrite(IN_1, HIGH);
        digitalWrite(IN_2, LOW);
        digitalWrite(IN_3, HIGH);
        digitalWrite(IN_4, LOW);
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_BACKWARD) == 0)
    {
        // BACKWARD
        digitalWrite(IN_1, LOW);
        digitalWrite(IN_2, HIGH);
        digitalWrite(IN_3, LOW);
        digitalWrite(IN_4, HIGH);
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_CCW) == 0)
    {
        // LEFT
        digitalWrite(IN_1, HIGH);
        digitalWrite(IN_2, LOW);
        digitalWrite(IN_3, LOW);
        digitalWrite(IN_4, HIGH);
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_CW) == 0)
    {
        // RIGHT
        digitalWrite(IN_1, LOW);
        digitalWrite(IN_2, HIGH);
        digitalWrite(IN_3, HIGH);
        digitalWrite(IN_4, LOW);
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_STOP) == 0)
    {
        digitalWrite(IN_1, LOW);
        digitalWrite(IN_2, LOW);
        digitalWrite(IN_3, LOW);
        digitalWrite(IN_4, LOW);
    }
}

void controlA076(const char *direction)
{
    if (DEBUG)
    {
        Serial.printf("-----direction:%s\n", direction);
    }
    delay(100);
    if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_STOP) == 0)
    {
        txdata[1] = Stop;
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_FORWARD) == 0)
    {
        txdata[1] = Forward;
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_BACKWARD) == 0)
    {
        txdata[1] = Backward;
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_LEFT) == 0)
    {
        txdata[1] = Contrarotate;
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_RIGHT) == 0)
    {
        txdata[1] = Clockwise;
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_CW) == 0)
    {
        txdata[1] = Clockwise;
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_CCW) == 0)
    {
        txdata[1] = Contrarotate;
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_NW) == 0)
    {
        txdata[1] = Top_Left;
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_NE) == 0)
    {
        txdata[1] = Top_Right;
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_SW) == 0)
    {
        txdata[1] = Bottom_Left;
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_SE) == 0)
    {
        txdata[1] = Bottom_Right;
    }
    Serial.write(txdata, 3);
    if (DEBUG)
    {
        Serial.print("-----txdata:");
        Serial.println((char *)txdata);
    }
    delay(100);
}

void sendControlData(const char *direction)
{
    if (strcmp(profile.c_str(), THING_PROFILE_ESP_LEGO_001) == 0)
    {
        if (DEBUG)
        {
            Serial.println("-----profile: Lego001");
        }
        controlLego001(direction);
    }
    else if (strcmp(profile.c_str(), THING_PROFILE_ESP_STANDARD_024) == 0)
    {
        if (DEBUG)
        {
            Serial.println("-----profile: S024");
        }
        controlS024(direction);
    }
    else if (strcmp(profile.c_str(), THING_PROFILE_ESP_ADVANCED_076) == 0)
    {
        if (DEBUG)
        {
            Serial.println("-----profile: A076");
        }
        controlA076(direction);
    }
}

void bootOrControl(const char *receivedData)
// void bootOrControl(String receivedData)
{
    StaticJsonDocument<512> doc;
    deserializeJson(doc, receivedData, DeserializationOption::NestingLimit(30));
    serializeJsonPretty(doc, Serial);
    Serial.println();

    if (doc["type"] != nullptr)
    {
        // Serial.printf("-----type: %s\n", doc["type"]);
        if (doc["type"] == "metric"){
            String data = doc["data"];
            if (data != nullptr){
                String server = doc["data"]["server"];
                if (server != nullptr){
                    String ssid = doc["data"]["server"]["ssid"];
                    String password = doc["data"]["server"]["password"];
                    const char* host = doc["data"]["server"]["host"];
                    Serial.printf("host: %s -------------------------",host);
                    Serial.println("");
                    int port = doc["data"]["server"]["port"].as<int>();
                    Serial.printf("port: %d -------------------------",port);
                    Serial.println("");
                    const char* path = doc["data"]["server"]["path"];
                    Serial.printf("path: %s -------------------------",path);
                    Serial.println("");
                    char buff[255];
                    sprintf(buff, "ws://%s:%d/%s", host, port, path);
                    Serial.printf("buff: %s", buff);
                    connectGateway(
                        ssid.c_str(),
                        password.c_str(),
                        host,
                        port,
                        buff
                    );
                }
            }
        }
    }
    // if (doc["ssid"] != nullptr)
    // {
    //     Serial.printf("-----decoded doc: %s\n", doc["host"]);

    //     String ssid = doc["ssid"].as<String>();
    //     String password = doc["password"].as<String>();
    //     String host = doc["host"].as<String>();
    //     int port = doc["port"].as<int>();
    //     char buff[255];
    //     String channelName = doc["channel_name"].as<String>();
    //     sprintf(buff, "/pang/ws/pub?channel=instant&name=%s&track=video&mode=bundle", channelName);
    //     profile = doc["profile"].as<String>();
    //     if (DEBUG)
    //     {
    //         Serial.println(ssid);
    //         Serial.println(password);
    //         Serial.println(host);
    //         Serial.println(port);
    //         Serial.println(buff);
    //         Serial.println(profile);
    //     }
    //     connectGateway(
    //         ssid.c_str(),
    //         password.c_str(),
    //         host.c_str(),
    //         port,
    //         buff);
    // }

    if (doc["direction"] != nullptr)
    {
        const char *direction = doc["direction"];
        if (DEBUG)
        {
            Serial.printf("-----control direction: %s\n", direction);
        }
        sendControlData(direction);
    }
}

class CPServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer) override
    {
        ble.connected = true;
        if (DEBUG)
        {
            Serial.println("-----BLE connected");
        }
    };

    void onDisconnect(BLEServer *pServer) override
    {
        ble.connected = false;
        if (DEBUG)
        {
            Serial.println("-----BLE disconnected");
        }
        pServer->getAdvertising()->start();
    }
};

class CPCharacteristicCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic) override
    {
        std::string value = pCharacteristic->getValue();
        if (value.empty())
        {
            if (DEBUG)
            {
                Serial.println("-----Empty received data");
            }
        }
        else
        {
            std::string value = pCharacteristic->getValue();
            const char *receviedData = value.c_str();
            Serial.printf("-----receviedData: %s\n", receviedData);
            Serial.printf("-----receviedData size: %d\n", strlen(receviedData));
            bootOrControl(receviedData);
        }
    }
};

void beginBLE()
{
    string bleName = BLE_NAME_PREFIX;
    bleName += "[";
    string macAddress = getMacAddress().c_str();
    bleName += macAddress;
    bleName += "]";
    if (DEBUG)
    {
        Serial.print("-----Starting BLE:");
        Serial.println(bleName.c_str());
    }
    bleName = "CoPlay[test030]";
    ble.begin(bleName, new CPServerCallbacks, new CPCharacteristicCallbacks);
}

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id)
    {
    case WEBSOCKET_EVENT_CONNECTED:
    {
        if (DEBUG)
        {
            Serial.println("-----WebSocket connected");
        }
        // sendVideoMime();
        ble.writeValue("PUB_SUCCESS");
        ble.end();
        break;
    }
    case WEBSOCKET_EVENT_DATA:
    {
        try
        {
            delay(50);
            const char *receviedData = (const char *)data->data_ptr;
            // String receviedData = (String)data->data_ptr;
            // Serial.printf("-----receviedData: %s\n", receviedData);
            // Serial.printf("-----receviedData size: %d\n", strlen(receviedData));
            bootOrControl(receviedData);
        }
        catch (int e)
        {
            Serial.printf("-----control exception: %d\n", e);
        }
        break;
    }
    case WEBSOCKET_EVENT_DISCONNECTED:
    {
        if (DEBUG)
        {
            Serial.println("-----WebSocket disconnected");
        }
        beginBLE();
        break;
    }
    }
}

void onEventsCallback(WebsocketsEvent event, String data)
{ // data == NULL, static_cast<int> event == 0.
    if (event == WebsocketsEvent::ConnectionOpened)
    {
        Serial.println("Connnection Opened");
        // HSerial.print("{'topic':'pub_video', 'data':{'result':'success'}}");
    }
    else if (event == WebsocketsEvent::ConnectionClosed)
    {
        Serial.println("Connection Closed");
        // HSerial.print("{'topic':'pub_video', 'data':{'result':'failed'}}");
    }
    else if (event == WebsocketsEvent::GotPing)
    {
        Serial.println("Got a Ping!");
    }
    else if (event == WebsocketsEvent::GotPong)
    {
        Serial.println("Got a Pong!");
    }
}

void connectGateway(const char *ssid, const char *password, const char *host, const uint16_t port, const char *path)
{
    if (!WiFi.isConnected())
    {
        /*
            와이파이 신호 세기 기준
            와이파이 신호 강도를 확인하기 전에 충분히 높은 세기는 어느 정도인지 확인해보자. 와이파이 신호 강도는 백분율이나 RSSI 값으로 표시한다.
            그 중 RSSI는 dBm 단위를 이용해 신호 강도를 나타낸다. RSSI의 값은 보통 음수이며, 0에 가까울수록 좋다.
            0에 가까울수록 신호가 강한 것이고, 멀수록 낮은 것이다. 다시 말해 마이너스를 빼고 보면 숫자가 낮을수록 신호가 강하고, 높을수록 신호가 약하다.
            주의할 점은 dBm은 선형적으로 증가하지 않는 것이다. RSSI값이 -53dBm로 -50dBm으로 변경됐다면, 3dBm 차이지만 신호 수준은 2배로 강해진다.
            반대라면 2배 약해진 것이다. RSSI 값 해석은 다음과 같이 하면 된다.
            -50dBm : 매우 우수한 신호 세기. 모바일 기기가 라우터 바로 옆에 있는 경우를 제외하고 보통 이보다 나은 수치를 보기 어렵다.
            -55에서 -60dBm 사이 : 강한 신호. 모바일 기기를 이용하기에 문제없으며, 스트리밍 서비스를 보기에 충분한 속도다.
            -70dBm : 약한 신호. 영상을 보기에는 충분치 않은 수치이지만 이메일 확인이나 웹 서핑하기에는 괜찮다.
            -80dBm : 매우 약한 신호로 기본적인 인터넷 이용을 하기 어려울 정도의 수치다.
            원문보기:
            https://www.itworld.co.kr/howto/256930#csidx51c2cf5365dd7d6a2291043e983431c
        */
        WiFi.begin(ssid, password);
        while (WiFiClass::status() != WL_CONNECTED)
        {
            delay(1000);
            if (DEBUG)
            {
                Serial.println((String) "-----RSSI : " + WiFi.RSSI() + " dB");
                Serial.print(".");
            }
        }
        if (DEBUG)
        {
            Serial.print("\n");
            Serial.println("-----WiFi connected");
        }
    }

    Serial.printf("-----new ssid, password: %s, %s\n", ssid, password);
    Serial.printf("-----new host, port: %s, %d\n", host, port);
    Serial.printf("-----new path, profile: %s, %s\n", path, profile);

    esp_websocket_client_config_t websocket_cfg = {
        .host = host,
        .port = port,
        .path = path,
        .task_prio = 1,
        .buffer_size = WEBSOCKET_BUFFER_SIZE_BYTE,
        .disable_pingpong_discon = true,
        .keep_alive_idle = 10,
    };

    client = esp_websocket_client_init(&websocket_cfg);

    ESP_ERROR_CHECK(
        esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, nullptr));

    esp_websocket_client_start(client);
}

esp_err_t initCameraConfig()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 16000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    return esp_camera_init(&config);
}

String getMacAddress()
{
    uint8_t baseMac[6];
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    char baseMacChr[18] = {0};
    sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    return String(baseMacChr);
}

void sendVideoMime()
{
    const char *mime = "image/jpeg;width=640;height=480;codecs=jpeg";
    esp_websocket_client_send_text(client, mime, (int)strlen(mime), portMAX_DELAY);
}

void sendCameraFrame()
{
    if (client && esp_websocket_client_is_connected(client))
    {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
        {
            if (DEBUG)
            {
                Serial.println("-----Camera capture failed");
            }
            esp_camera_fb_return(fb);
            return;
        }
        esp_websocket_client_send_bin(client, (char *)fb->buf, fb->len, portMAX_DELAY);
        if (DEBUG)
        {
            // Serial.printf("Data sent: %d\n", fb->len);
            // Serial.printf("Data sent: %d\n", builder.GetSize());
        }
        esp_camera_fb_return(fb);

        // now = millis();
        if (flag_mime)
        {
            flag_mime = false;
            // prevous = now;
            sendVideoMime();
        }
    }
}

void initCamera()
{
    esp_err_t err = initCameraConfig();
    if (err != ESP_OK)
    {
        if (DEBUG)
        {
            Serial.printf("-----Camera init failed with error 0x%x", err);
        }
        return;
    }

    cam_sensor = esp_camera_sensor_get();
    cam_sensor->set_framesize(cam_sensor, FRAMESIZE_VGA);
    cam_sensor->set_quality(cam_sensor, 15);
    cam_sensor->set_gainceiling(cam_sensor, static_cast<gainceiling_t>(0));
}

void setup()
{
    delay(500);
    psramInit();
    Serial.begin(115200);
    Serial.println((String) "Memory available in PSRAM : " + ESP.getFreePsram());

    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // Setup pinmode for R024
    pinMode(IN_1, OUTPUT);
    pinMode(IN_2, OUTPUT);
    pinMode(IN_3, OUTPUT);
    pinMode(IN_4, OUTPUT);
    digitalWrite(IN_1, LOW);
    digitalWrite(IN_2, LOW);
    digitalWrite(IN_3, LOW);
    digitalWrite(IN_4, LOW);

    // Begin BLE Server
    beginBLE();

    // Initialize Camera
    initCamera();

    // test();
}

void loop()
{
    sendCameraFrame();
    delay(10);
}