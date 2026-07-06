#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <String.h>
#include <WString.h>
#include "Preferences.h"
#include "ArduinoJson.h"

#define VERSION 0.2
#define DEBUG_MODE //Display debug messages in serial. Comment out to disable debug messages
#define TAPDISPLAY_BOARD 0.2

#define CFG_SSID_SIZE     32
#define CFG_PSK_SIZE      64
#define CFG_HOSTNAME_SIZE 16

//MQTT
 #define CFG_MQTT_PROTOCOL_SIZE     5
 #define CFG_MQTT_HOST_SIZE         32
  #define CFG_MQTT_USER_SIZE         32
  #define CFG_MQTT_PASSWORD_SIZE     32
  #define CFG_MQTT_DEFAULT_ACTIVATED false
  #define CFG_MQTT_DEFAULT_PROTOCOL  PSTR("mqtt")
  #define CFG_MQTT_DEFAULT_AUTH      false
  #define CFG_MQTT_DEFAULT_HOST      PSTR("mqtt.local")
  #define CFG_MQTT_DEFAULT_PORT      1883


#define DEFAULT_LED_BRIGHTNESS  50                // 50%

// Port pour l'OTA
#define DEFAULT_OTA_PORT     8266
#define DEFAULT_OTA_AUTH     PSTR("OTA_TapDisplay")
//#define DEFAULT_OTA_AUTH     ""


// Web Interface Configuration Form field names
#define CFG_FORM_SSID     PSTR("ssid")
#define CFG_FORM_PSK      PSTR("psk")
#define CFG_FORM_HOST     PSTR("host")
#define CFG_FORM_AP_PSK   PSTR("ap_psk")
#define CFG_FORM_OTA_AUTH PSTR("ota_auth")
#define CFG_FORM_OTA_PORT PSTR("ota_port")

//Colors for display
// Newer "GFX Library for Arduino" releases only expose RGB565_xxx color
// macros (bare BLACK/WHITE/... were dropped). Alias the ones we use so the
// rest of the code doesn't need to change.
#define BLACK   RGB565_BLACK
#define WHITE   RGB565_WHITE
#define RED     RGB565_RED
#define GREEN   RGB565_GREEN
#define BLUE    RGB565_BLUE
#define YELLOW  RGB565_YELLOW
#define DARKGREY RGB565_DARKGREY

#define BACKGROUND BLACK
#define MARK_COLOR WHITE
#define SUBMARK_COLOR DARKGREY // LIGHTGREY
#define HOUR_COLOR WHITE
#define MINUTE_COLOR BLUE // LIGHTGREY
#define SECOND_COLOR RED
#define HOP_COLOR GREEN

#define SIXTIETH 0.016666667
#define TWELFTH 0.08333333
#define SIXTIETH_RADIAN 0.10471976
#define TWELFTH_RADIAN 0.52359878
#define RIGHT_ANGLE_RADIAN 1.5707963
#define RIGHT_STOP 5.4977 //7PI/4
#define LEFT_STOP 3.9 //5PI/4

// GC9A01 TFT pins - JCZN ESP32-2424S012 board (ESP32-C3 + 1.28" round touch
// display). RST is not broken out on this board (tied to the module's own
// reset), so it's left unconnected and the panel is reset over SPI instead.
#define TFT_BL 3
#define TFT_DC 2
#define TFT_CS 10
#define TFT_SCK 6
#define TFT_MOSI 7
#define TFT_MISO -1
#define TFT_RST -1

// CST816 capacitive touch controller (I2C), same board
#define TOUCH_SDA 4
#define TOUCH_SCL 5
#define TOUCH_RST 1
#define TOUCH_INT 0
#define TOUCH_I2C_ADDR 0x15

// ---------------------------------------------------------------------------
// BLE identity & protocol (weight comes from the remote scale board now,
// there is no HX711 wired to this board anymore)
// ---------------------------------------------------------------------------
// These MUST match scale-esp32c6/include/config.h
#define BLE_DEVICE_NAME   "BeerTapScale"
#define BLE_SERVICE_UUID  "a1f4e100-3b7a-4f56-9d1b-2f6a9c5e10a0"
#define BLE_WEIGHT_UUID   "a1f4e101-3b7a-4f56-9d1b-2f6a9c5e11a0" // float32 grams, read+notify
#define BLE_COMMAND_UUID  "a1f4e102-3b7a-4f56-9d1b-2f6a9c5e12a0" // uint8 command, write

#define BLE_CMD_TARE 0x01

// Consider the last weight reading stale after this delay (ms) - used to
// show a "scale offline" indicator on screen.
#define BLE_WEIGHT_STALE_MS 15000
// How often to retry scanning/connecting to the scale when disconnected (ms)
#define BLE_RECONNECT_PERIOD_MS 5000

// Config saved into eeprom
// 1024 bytes total including CRC
typedef struct
{
  char ssid[CFG_SSID_SIZE+1];                       // SSID
  char psk[CFG_PSK_SIZE+1];                         // Pre shared key
  char host[CFG_HOSTNAME_SIZE+1];                   // Hostname
  char ap_psk[CFG_PSK_SIZE+1];                      // Access Point Pre shared key
  char ota_auth[CFG_PSK_SIZE+1];                    // OTA Authentication password
  uint32_t config;                                  // Bit field register
  uint16_t ota_port;                                // OTA port
  uint8_t  led_bright;                              // RGB Led brightness 0-255
  uint16_t oled_type;                               // Display OLED type (1306 or 1106)
  uint8_t  filler[104];                             // in case adding data in config avoiding loosing current conf by bad crc
  #ifdef MOD_EMONCMS
  _emoncms emoncms;                                 // Emoncms configuration
  #else
  uint8_t  filler_emoncms[128];
  #endif
  #ifdef MOD_JEEDOM
  _jeedom  jeedom;                                  // jeedom configuration
  #else
  uint8_t  filler_jeedom[256];
  #endif
  #ifdef MOD_MQTT
    _mqtt    mqtt;                                  // MQTT configuration
  #else
    uint8_t  filler_mqtt[128];
  #endif
  uint8_t  filler1[128];                            //Another filler in case we need more
  uint16_t crc;
} _Config;


// Exported variables/object instancied in main sketch
// ===================================================
extern _Config config;



// Declared exported function from route.cpp
// ===================================================
bool readConfig(bool clear_on_error=true);
bool saveConfig(void);
void resetConfig(void);
void showConfig(void);

extern unsigned long uptime;

extern String getBeerName();
extern void setBeerName(String value);

extern float getBeerSg();
extern void setBeerSg(float value);

extern int getBeerIBU();
extern void setBeerIBU(int value);

extern int getBeerEBC();
extern void setBeerEBC(int value);

extern float getBeerABV();
extern void setBeerABV(float value);

extern String getFileForEBC();
#endif
