#ifndef __CONFIG_H__
#define __CONFIG_H__

#define VERSION 0.1
#define DEBUG_MODE // Display debug messages in serial. Comment out to disable.

// ---------------------------------------------------------------------------
// HX711 wiring
// ---------------------------------------------------------------------------
// GPIO4/5/8/9/15 are strapping pins on the ESP32-C6 (used at boot) and should
// be avoided. GPIO2/3 are free general purpose pins on the DevKitM-1 board.
#define HX711_DOUT_PIN 2 // mcu -> HX711 DOUT pin
#define HX711_SCK_PIN  3 // mcu -> HX711 SCK pin

// Needs to be calibrated for each individual load cell / scale assembly.
#define SCALE_CALIBRATION_FACTOR 20710

// How often the weight is measured and pushed over BLE (ms)
#define WEIGHT_UPDATE_PERIOD_MS 2000

// ---------------------------------------------------------------------------
// BLE identity & protocol
// ---------------------------------------------------------------------------
// These UUIDs and the device name MUST match the ones used in
// display-esp32/include/config.h so the display can find and talk to
// this scale.
#define BLE_DEVICE_NAME   "BeerTapScale"
#define BLE_SERVICE_UUID  "a1f4e100-3b7a-4f56-9d1b-2f6a9c5e10a0"
#define BLE_WEIGHT_UUID   "a1f4e101-3b7a-4f56-9d1b-2f6a9c5e11a0" // float32 grams, read+notify
#define BLE_COMMAND_UUID  "a1f4e102-3b7a-4f56-9d1b-2f6a9c5e12a0" // uint8 command, write

// Command byte values accepted on BLE_COMMAND_UUID
#define BLE_CMD_TARE 0x01

#endif
