#include <Arduino.h>
#include "ble_client.h"
#include "config.h"

#include <NimBLEDevice.h>

static NimBLEClient *pClient = nullptr;
static const NimBLEAdvertisedDevice *scaleDevice = nullptr;
static NimBLERemoteCharacteristic *pWeightChar = nullptr;
static NimBLERemoteCharacteristic *pCommandChar = nullptr;

static volatile bool foundScale = false;
static volatile bool connected = false;
static unsigned long lastReconnectAttempt = 0;

static volatile float lastWeight = 0.0f;
static volatile unsigned long lastWeightMillis = 0;

// ---------------------------------------------------------------------------
// Scan callback: filters advertised devices down to our scale by service UUID
// ---------------------------------------------------------------------------
class ScaleScanCallbacks : public NimBLEScanCallbacks
{
  void onResult(const NimBLEAdvertisedDevice *advertisedDevice)
  {
    if (advertisedDevice->isAdvertisingService(NimBLEUUID(BLE_SERVICE_UUID)))
    {
      Serial.println("Scale found, stopping scan");
      NimBLEDevice::getScan()->stop();
      scaleDevice = advertisedDevice;
      foundScale = true;
    }
  }
};

static void weightNotifyCallback(NimBLERemoteCharacteristic *characteristic,
                                  uint8_t *data, size_t length, bool isNotify)
{
  if (length == sizeof(float))
  {
    float weight;
    memcpy(&weight, data, sizeof(float));
    lastWeight = weight;
    lastWeightMillis = millis();
  }
}

class ScaleClientCallbacks : public NimBLEClientCallbacks
{
  void onDisconnect(NimBLEClient *client, int reason)
  {
    connected = false;
    Serial.println("Scale disconnected");
  }
};

static bool connectToScale()
{
  Serial.println("Connecting to scale...");

  pClient = NimBLEDevice::createClient();
  pClient->setClientCallbacks(new ScaleClientCallbacks(), false);

  if (!pClient->connect(scaleDevice))
  {
    Serial.println("Failed to connect to scale");
    return false;
  }

  NimBLERemoteService *pService = pClient->getService(BLE_SERVICE_UUID);
  if (pService == nullptr)
  {
    Serial.println("Scale service not found");
    pClient->disconnect();
    return false;
  }

  pWeightChar = pService->getCharacteristic(BLE_WEIGHT_UUID);
  pCommandChar = pService->getCharacteristic(BLE_COMMAND_UUID);

  if (pWeightChar == nullptr || !pWeightChar->canNotify())
  {
    Serial.println("Weight characteristic not usable");
    pClient->disconnect();
    return false;
  }

  pWeightChar->subscribe(true, weightNotifyCallback);

  connected = true;
  Serial.println("Connected to scale");
  return true;
}

void ble_client_init()
{
  NimBLEDevice::init("");

  NimBLEScan *pScan = NimBLEDevice::getScan();
  pScan->setScanCallbacks(new ScaleScanCallbacks(), false);
  pScan->setInterval(45);
  pScan->setWindow(15);
  pScan->setActiveScan(true);
  pScan->start(5, false);
}

void ble_client_loop()
{
  if (foundScale && !connected)
  {
    foundScale = false;
    if (!connectToScale())
    {
      // will retry on the next reconnect tick below
    }
  }

  if (!connected && !foundScale)
  {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > BLE_RECONNECT_PERIOD_MS)
    {
      lastReconnectAttempt = now;
      Serial.println("Scanning for scale...");
      NimBLEDevice::getScan()->start(5, false);
    }
  }
}

void ble_client_send_tare()
{
  if (connected && pCommandChar != nullptr && pCommandChar->canWrite())
  {
    uint8_t cmd = BLE_CMD_TARE;
    pCommandChar->writeValue(&cmd, 1, false);
  }
}

bool ble_client_is_connected()
{
  return connected;
}

float ble_client_get_weight()
{
  return lastWeight;
}

unsigned long ble_client_last_update()
{
  return lastWeightMillis;
}
