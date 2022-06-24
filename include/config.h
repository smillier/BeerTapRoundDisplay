#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <String.h>
#include <WString.h>
#define VERSION 0.1
#define DEBUG_MODE //Display debug messages in serial. Comment out to disable debug messages

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

extern String beer_name;
 String getBeerName();
 void setBeerName(String value);
#endif
