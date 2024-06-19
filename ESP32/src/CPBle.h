#ifndef CP_BLE_H
#define CP_BLE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLESecurity.h>
#include <iostream>
#include <functional>
#include <utility>

using namespace std;

class CPBle
{
public:
    BLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *pTxCharacteristic, *pRxCharacteristic;
    bool connected;
    BLESecurity *pSecurity;

    void begin(string name, BLEServerCallbacks *serverCallbacks, BLECharacteristicCallbacks *characteristicCallbacks);
    void end() const;
    void writeValue(string value) const;
private:
    const string service_uuid = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    const string rx_uuid = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
    const string tx_uuid = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
};

#endif