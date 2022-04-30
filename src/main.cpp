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


void setup(void)
{
    initialize();
}

void loop()
{
    display_init();
     delayMicroseconds(5000000);
    //ArduinoOTA.handle();    
}









