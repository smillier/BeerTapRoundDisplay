#include <Arduino.h>
#include "weightmeasure.h"
#include "config.h"
#include "ble_client.h"

// The HX711 is no longer wired to this board: the weight now comes from the
// remote scale board (ESP32-C6 + HX711) over BLE. See ble_client.cpp.

void scale_setup()
{
  ble_client_init();
}

void scale_calibrate()
{
  ble_client_send_tare();
}

float getWeight()
{
  return ble_client_get_weight();
}

float getVolume()
{
  float sg = getBeerSg();
  if (sg == 0.0)
    return 0.0;
  float volume = getWeight() / sg;
  return volume;
}

bool isScaleConnected()
{
  return ble_client_is_connected();
}

bool isWeightStale()
{
  return (millis() - ble_client_last_update()) > BLE_WEIGHT_STALE_MS;
}
