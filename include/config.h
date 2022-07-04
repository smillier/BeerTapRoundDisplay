#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <String.h>
#include <WString.h>
#include "Preferences.h"
#include "ArduinoJson.h"

#define VERSION 0.1
#define DEBUG_MODE //Display debug messages in serial. Comment out to disable debug messages
#define TAPDISPLAY_BOARD 0.1

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

//GC9A01 TFT pins:
#define TFT_BL 21
#define TFT_DC 4
#define TFT_CS 5
#define TFT_SCK 18
#define TFT_MOSI 23
#define TFT_MISO -1
#define TFT_RST 16

//HX711 pins:
#define HX711_dout 25 //mcu > HX711 dout pin
#define HX711_sck 12 //mcu > HX711 sck pin
#define SCALE_CALIBRATION_FACTOR 20710 //needs to be adapted to each scale

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
