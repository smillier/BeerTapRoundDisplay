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
#include "touch.h"
#include <string>
#include <SPIFFS.h>
#include <ArduinoOTA.h>
#include <ElegantOTA.h>
#include "Preferences.h"

void setup(void)
{
    initialize();
    initWebServer();
    touch_init();
}

void loop()
{
    ble_client_loop();
    refresh_display();
    ArduinoOTA.handle();
    ElegantOTA.loop();

    // Same ~2s cadence as before for the display/BLE/OTA work above, but
    // polled in small steps so touch taps aren't missed in between.
    for (int i = 0; i < 20; i++)
    {
        touch_loop();
        delay(100);
    }
}
