#ifndef __DISPLAY_H__
#define __DISPLAY_H__
#include <cstdint>
extern void display_init();
extern void refresh_display();
//extern void bmpDrawCallback(int16_t );
extern void  draw_round_clock_mark(int16_t innerR1, int16_t outerR1, int16_t innerR2, int16_t outerR2, int16_t innerR3, int16_t outerR3);
extern void write_cache_pixel(int16_t x, int16_t y, int16_t color, bool cross_check_second, bool cross_check_hour);
extern void draw_and_erase_cached_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color, int16_t *cache, int16_t cache_len, bool cross_check_second, bool cross_check_hour);
extern void redraw_hands_cached_draw_and_erase();
//extern void write_cache_pixel(int16_t x, int16_t y, int16_t color, bool cross_check_second, bool cross_check_hour);
extern unsigned int rainbow(uint8_t value);
extern int ringMeter(int value, int vmin, int vmax, int x, int y, int r, char *units, uint8_t scheme);
#endif