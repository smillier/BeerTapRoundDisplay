#include "weightmeasure.h"
#include "config.h"

#include "HX711.h"

HX711 scale;

void scale_setup()
{
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(SCALE_CALIBRATION_FACTOR);
  // tare()/get_units() otherwise block forever if the HX711 never pulls
  // DOUT low (not wired yet, loose connection...) - skip rather than
  // hang setup() before BLE ever gets a chance to start advertising.
  if (scale.wait_ready_timeout(1000))
    scale.tare();
  else
    Serial.println("HX711 not responding, skipping tare (load cell not connected?)");
}

void scale_tare()
{
  if (scale.wait_ready_timeout(1000))
    scale.tare();
}

float getWeight()
{
  if (!scale.wait_ready_timeout(1000))
    return 0.0f;
  return scale.get_units(2);
}
