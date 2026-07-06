/****************************************************************
 *  BeerTap Round Display
 *  https://github.com/smillier/BeerTapRoundDisplay
 *  Version: 0.2 - split board: weight comes from a remote ESP32-C6
 *  scale over BLE instead of a local HX711.
*****************************************************************/
#include "config.h"
#include "setup.h"
#include "weightmeasure.h"
#include "ble_client.h"
#include "FS.h"
#include "display.h"
#include <string>
#include <SPIFFS.h>
#include <ArduinoOTA.h>
#include <ElegantOTA.h>
#include "Preferences.h"

void setup(void)
{
    initialize();
    initWebServer();
}

void loop()
{
    ble_client_loop();
    refresh_display();
    delayMicroseconds(2000000); //2000000
    ArduinoOTA.handle();
    ElegantOTA.loop();
}
