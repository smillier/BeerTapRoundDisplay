#include <Arduino.h>
#include "touch.h"
#include "config.h"
#include "weightmeasure.h"
#include <CST816S.h>

static CST816S touch(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_INT);

// Debounce: ignore further taps for a short while after one is handled, the
// controller can report the same gesture a few times in a row otherwise.
#define TOUCH_DEBOUNCE_MS 400
static unsigned long lastTapMillis = 0;

void touch_init()
{
  touch.begin();
}

void touch_loop()
{
  if (!touch.available())
    return;

  // A long-press is required (not a plain tap) so tare isn't triggered by
  // accidentally brushing the screen - see the README's attention note.
  if (touch.data.gestureID != LONG_PRESS)
    return;

  unsigned long now = millis();
  if (now - lastTapMillis < TOUCH_DEBOUNCE_MS)
    return;
  lastTapMillis = now;

  Serial.println("Touch long-press: tare");
  scale_calibrate();
}
