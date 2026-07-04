#include "weightmeasure.h"
#include "config.h"

#include "HX711.h"

HX711 scale;

void scale_setup()
{
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(SCALE_CALIBRATION_FACTOR);
  scale.tare();
}

void scale_tare()
{
  scale.tare();
}

float getWeight()
{
  return scale.get_units(2);
}
