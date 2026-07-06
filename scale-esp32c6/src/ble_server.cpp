#include <Arduino.h>
#include "ble_server.h"
#include "config.h"
#include "weightmeasure.h"

#include <NimBLEDevice.h>

static NimBLECharacteristic *pWeightChar = nullptr;
static NimBLECharacteristic *pCommandChar = nullptr;
static volatile bool clientConnected = false;

class ScaleServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo)
  {
    clientConnected = true;
    Serial.println("Display connected");
  }

  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason)
  {
    clientConnected = false;
    Serial.println("Display disconnected, resuming advertising");
    NimBLEDevice::startAdvertising();
  }
};

class ScaleCommandCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(NimBLECharacteristic *pChar, NimBLEConnInfo &connInfo)
  {
    std::string value = pChar->getValue();
    if (!value.empty() && (uint8_t)value[0] == BLE_CMD_TARE)
    {
      Serial.println("Tare requested by display");
      scale_tare();
    }
  }
};

void ble_server_init()
{
  NimBLEDevice::init(BLE_DEVICE_NAME);

  NimBLEServer *pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ScaleServerCallbacks());

  NimBLEService *pService = pServer->createService(BLE_SERVICE_UUID);

  pWeightChar = pService->createCharacteristic(
      BLE_WEIGHT_UUID,
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  pCommandChar = pService->createCharacteristic(
      BLE_COMMAND_UUID,
      NIMBLE_PROPERTY::WRITE);
  pCommandChar->setCallbacks(new ScaleCommandCallbacks());

  pService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
  pAdvertising->enableScanResponse(true);
  pAdvertising->start();

  Serial.println("BLE server advertising as " BLE_DEVICE_NAME);
}

void ble_notify_weight(float grams)
{
  if (pWeightChar == nullptr)
    return;

  pWeightChar->setValue((uint8_t *)&grams, sizeof(grams));
  if (clientConnected)
    pWeightChar->notify();
}

bool ble_server_has_client()
{
  return clientConnected;
}
