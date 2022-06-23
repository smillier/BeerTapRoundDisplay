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

void setup(void)
{
    //Serial.begin(115200);
    initialize();
}

void loop()
{
    refresh_display();
    //redraw_hands_cached_draw_and_erase();
   delayMicroseconds(5000000);
    ArduinoOTA.handle();    
}









