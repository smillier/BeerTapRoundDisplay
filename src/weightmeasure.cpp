#include "weightmeasure.h"
#include <HX711_ADC.h>
#include <EEPROM.h>
#include "config.h"

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
long t;
String value = "Load cell: 0.0";

float getWeight()
{
    static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity
     float i =0.0;
  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      i = LoadCell.getData();
      newDataReady = 0;
      t = millis();
      value = "Poids: " + String(i) + "G";
      #ifdef DEBUG_MODE
      Serial.println(value);
      #endif
    }
  }
    return i;
}