#include "weightmeasure.h"
#include "config.h"

#include "HX711.h"

HX711 scale;

void scale_setup()
{
  scale.begin(HX711_dout, HX711_sck);
  //scale.tare();
  scale.set_scale(SCALE_CALIBRATION_FACTOR);
}

void scale_calibrate()
{
  
}

float getWeight()
{
  return scale.get_units(2);
}

float getVolume()
{
  float sg = getBeerSg();
  if (sg == 0.0)
    return 0.0;
  float volume = getWeight() / sg;
  return volume;
}