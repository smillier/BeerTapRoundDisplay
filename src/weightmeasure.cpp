#include "weightmeasure.h"
#include "config.h"

#include "HX711.h"

HX711 scale;

void scale_setup()
{
  scale.begin(HX711_dout, HX711_sck);
  scale.tare();
}

void scale_calibrate()
{
   while(!Serial.available());
  while(Serial.available()) Serial.read();
  scale.calibrate_scale(750, 5);
}

float getWeight()
{
  return scale.get_units(10);
}