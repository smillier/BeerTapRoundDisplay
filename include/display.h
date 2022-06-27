#ifndef __DISPLAY_H__
#define __DISPLAY_H__
#include <cstdint>
#include <String.h>
#include <WString.h>

extern void display_init();
extern void refresh_display();
void updateMeter();
void clearDynamicZones();
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, char *units, uint8_t scheme);

#endif