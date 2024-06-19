#include "CPBle.h"

#include <utility>

void CPBle::begin(string name, BLEServerCallbacks *serverCallbacks, BLECharacteristicCallbacks *characteristicCallbacks)
{
    BLEDevice::init(std::move(name));
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(serverCallbacks);

    pService = pServer->createService(service_uuid);
    pTxCharacteristic = pService->createCharacteristic(
        tx_uuid,
        BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->setCallbacks(characteristicCallbacks);
    pTxCharacteristic->addDescriptor(new BLE2902());

    pRxCharacteristic = pService->createCharacteristic(
        rx_uuid,
        BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
    pRxCharacteristic->setCallbacks(characteristicCallbacks);
    pRxCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    pServer->getAdvertising()->addServiceUUID(service_uuid);
    pServer->getAdvertising()->setScanResponse(false);
    pServer->getAdvertising()->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pServer->getAdvertising()->setMinPreferred(0x12);

    pServer->getAdvertising()->start();
    connected = false;
    //    Serial.println("-----Start advertising");
}

void CPBle::end() const
{
    pServer->getAdvertising()->stop();
    pService->stop();
    BLEDevice::deinit(true);
}

void CPBle::writeValue(string value) const
{
    // Serial.printf("-----notify value : %s\n", value.c_str());
    pTxCharacteristic->setValue(std::move(value));
    pTxCharacteristic->notify();
    // Serial.printf("-----notify success");
}
