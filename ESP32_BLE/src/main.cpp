//
// Created by Park Juchan on 2023/03/31.
// Modified by Kim Dongjin on 2023/05/15.
//
#define CAMERA_MODEL_AI_THINKER

#include "CPBle.h"
#include "local_msg_generated.h"
#include "esp_camera.h"
#include "esp_wifi.h"
#include "string"
#include "WiFi.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define DIRECTION_KEY_VALUE_DIRECTION_STOP "stop"
#define DIRECTION_KEY_VALUE_DIRECTION_FORWARD "forward"
#define DIRECTION_KEY_VALUE_DIRECTION_BACKWARD "backward"
#define DIRECTION_KEY_VALUE_DIRECTION_LEFT "left"
#define DIRECTION_KEY_VALUE_DIRECTION_RIGHT "right"
#define DIRECTION_KEY_VALUE_DIRECTION_CW "cw"
#define DIRECTION_KEY_VALUE_DIRECTION_CCW "ccw"
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

#define DEBUG false

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
unsigned long now = 0;
unsigned long prevous = 0;
flatbuffers::FlatBufferBuilder builder(1024);
int prevSecond = -1;
int fps = 0;
int prevMinute = -1;
int fpm = 0;
String profile = THING_PROFILE_ESP_STANDARD_024;
// String profile = THING_PROFILE_ESP_ADVANCED_076;
// String profile = THING_PROFILE_ESP_LEGO_001;

using namespace CoPlay::LocalMessage;

void controlLego001(const ControlData *controlData)
{
    if (DEBUG)
    {
        Serial.print("-----R001 direction:");
        Serial.print(controlData->direction()->c_str());
        Serial.print(", value:");
        Serial.println(controlData->value());
    }
    const char *direciton = controlData->direction()->c_str();
    Serial.write(direciton);
    Serial.write('$');
}

void controlS024(const ControlData *controlData)
{
    pinMode(2, OUTPUT);
    pinMode(13, OUTPUT);
    pinMode(14, OUTPUT);
    pinMode(15, OUTPUT);
    if (strcmp(controlData->direction()->c_str(), DIRECTION_KEY_VALUE_DIRECTION_FORWARD) == 0)
    {
        // FORWARD
        digitalWrite(13, LOW);
        digitalWrite(15, HIGH);
        digitalWrite(14, LOW);
        digitalWrite(2, HIGH);
    }
    else if (strcmp(controlData->direction()->c_str(), DIRECTION_KEY_VALUE_DIRECTION_BACKWARD) == 0)
    {
        // BACKWARD
        digitalWrite(13, HIGH);
        digitalWrite(15, LOW);
        digitalWrite(14, HIGH);
        digitalWrite(2, LOW);
    }
    else if (strcmp(controlData->direction()->c_str(), DIRECTION_KEY_VALUE_DIRECTION_LEFT) == 0)
    {
        // LEFT
        digitalWrite(13, HIGH);
        digitalWrite(15, LOW);
        digitalWrite(14, LOW);
        digitalWrite(2, HIGH);
    }
    else if (strcmp(controlData->direction()->c_str(), DIRECTION_KEY_VALUE_DIRECTION_RIGHT) == 0)
    {
        // RIGHT
        digitalWrite(13, LOW);
        digitalWrite(15, HIGH);
        digitalWrite(14, HIGH);
        digitalWrite(2, LOW);
    }
    else if (strcmp(controlData->direction()->c_str(), DIRECTION_KEY_VALUE_DIRECTION_STOP) == 0)
    {
        digitalWrite(13, LOW);
        digitalWrite(15, LOW);
        digitalWrite(14, LOW);
        digitalWrite(2, LOW);
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
        // txdata[1] = Turn_Left;
        txdata[1] = Contrarotate;
    }
    else if (strcmp(direction, DIRECTION_KEY_VALUE_DIRECTION_RIGHT) == 0)
    {
        // txdata[1] = Turn_Right;
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

void sendControlData(const ControlData *controlData)
{
    if (strcmp(profile.c_str(), THING_PROFILE_ESP_LEGO_001) == 0)
    {
        if (DEBUG)
        {
            Serial.println("-----profile: Lego001");
        }
        controlLego001(controlData);
    }
    else if (strcmp(profile.c_str(), THING_PROFILE_ESP_STANDARD_024) == 0)
    {
        if (DEBUG)
        {
            Serial.println("-----profile: S024");
        }
        controlS024(controlData);
    }
    else if (strcmp(profile.c_str(), THING_PROFILE_ESP_ADVANCED_076) == 0)
    {
        if (DEBUG)
        {
            Serial.println("-----profile: A076");
        }
        controlA076(controlData->direction()->c_str());
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
            flatbuffers::Verifier verifier((uint8_t *)value.data(), value.length());
            if (verifier.VerifyBuffer<CoPlay::LocalMessage::LocalMessage>(nullptr))
            {
                if (DEBUG)
                {
                    Serial.println("-----Data is LocalMessage");
                }
                auto localMessage = GetLocalMessage(reinterpret_cast<const uint8_t *>(value.data()));
                if (localMessage->data_type() == Data_ControlData)
                {
                    if (DEBUG)
                    {
                        Serial.println("-----Data Type is ControlData");
                    }
                    sendControlData(localMessage->data_as_ControlData());
                }
            }
            else
            {
                if (DEBUG)
                {
                    Serial.println("-----Data is not LocalMessage");
                }
            }
        }
    }
};

String getMacAddress()
{
    uint8_t baseMac[6];
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    char baseMacChr[18] = {0};
    sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    return String(baseMacChr);
}

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
    ble.begin(bleName, new CPServerCallbacks, new CPCharacteristicCallbacks);
}

void setup()
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);

    // Setup pinmode for R024
    pinMode(2, OUTPUT);
    pinMode(13, OUTPUT);
    pinMode(14, OUTPUT);
    pinMode(15, OUTPUT);
    digitalWrite(2, LOW);
    digitalWrite(13, LOW);
    digitalWrite(14, LOW);
    digitalWrite(15, LOW);

    // Begin BLE Server
    beginBLE();
}

void loop()
{
}
