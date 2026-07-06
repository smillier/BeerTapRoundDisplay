/****************************************************************
 *  BeerTap Round Display - Scale node (ESP32-C6 + HX711)
 *  https://github.com/smillier/BeerTapRoundDisplay
 *
 *  Reads the keg weight from the HX711 load cell amplifier and
 *  broadcasts it over BLE to the remote display board.
 *  No WiFi/display/OTA on this node: it only needs power + BLE.
 *****************************************************************/
#include <Arduino.h>
#include "config.h"
#include "weightmeasure.h"
#include "ble_server.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("BeerTap scale node starting...");

  scale_setup();
  ble_server_init();
}

void loop()
{
  float weight = getWeight();

#ifdef DEBUG_MODE
  Serial.printf("Weight: %.2f g (client %s)\n", weight,
                ble_server_has_client() ? "connected" : "disconnected");
#endif

  ble_notify_weight(weight);
  delay(WEIGHT_UPDATE_PERIOD_MS);
}
