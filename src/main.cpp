/****************************************************************
 *  BeerTap Round Display
 *  https://github.com/smillier/BeerTapRoundDisplay
 *  Version: 0.1
*****************************************************************/
#include "config.h"
#include "setup.h"
#include "weightmeasure.h"
#include "FS.h"
#include "display.h"
#include <string>
#include <SPIFFS.h>
#include <ArduinoOTA.h>
#include "Preferences.h"

//Preferences preference;


void setup(void)
{
    //Serial.begin(115200);
    
    initialize();
    initWebServer();
}

void loop()
{
   // Serial.println(Config.beer_name);
    refresh_display();
    //redraw_hands_cached_draw_and_erase();
   delayMicroseconds(2000000); //2000000
    ArduinoOTA.handle();    
}









