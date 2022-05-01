#include "config.h"
#include "display.h"
#include "weightmeasure.h"
#include <FS.h>
#include <SPIFFS.h>
#include <Arduino_GFX_Library.h>
#include "BmpClass.h"

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO, VSPI /* spi_num */);

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *gfx = new Arduino_GC9A01(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

static int16_t w, h, center;
static int16_t hHandLen, mHandLen, sHandLen, markLen;
static float sdeg, mdeg, hdeg;
static int16_t osx = 0, osy = 0, omx = 0, omy = 0, ohx = 0, ohy = 0; // Saved H, M, S x & y coords
static int16_t nsx, nsy, nmx, nmy, nhx, nhy;                         // H, M, S x & y coords
static int16_t xMin, yMin, xMax, yMax;                               // redraw range
static int16_t hh, mm, ss;
static unsigned long targetTime; // next action time

static int16_t *cached_points;
static uint16_t cached_points_idx = 0;
static int16_t *last_cached_point;
int keg_level = 0;
#include "BmpClass.h"
static BmpClass bmpClass;

// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

void display_init()
{
     gfx->begin();
    gfx->fillScreen(BACKGROUND);
    Serial.begin(9600);
    
  
#ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
#endif

    // init LCD constant
    w = gfx->width();
    h = gfx->height();
    if (w < h)
    {
        center = w / 2;
    }
    else
    {
        center = h / 2;
    }
    hHandLen = center * 3 / 8;
    mHandLen = center * 2 / 3;
    sHandLen = center * 5 / 6;
    markLen = sHandLen / 6;
    cached_points = (int16_t *)malloc((hHandLen + 1 + mHandLen + 1 + sHandLen + 1) * 2 * 2);

    // Draw 60 clock marks
    draw_round_clock_mark(
    // draw_square_clock_mark(
        center - markLen, center,
        center - (markLen * 2 / 3), center,
        center - (markLen / 2), center);
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(uint8_t value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  uint8_t red = 0; // Red is the top 5 bits of a 16 bit colour value
  uint8_t g = 0;// Green is the middle 6 bits
  uint8_t b = 0; // Blue is the bottom 5 bits

  uint8_t quadrant = value / 32;

  if (quadrant == 0) {
    b = 31;
    g = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    b = 31 - (value % 32);
    g = 63;
    red = 0;
  }
  if (quadrant == 2) {
    b = 0;
    g = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    b = 0;
    g = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (g << 5) + b;
}


// pixel drawing callback
static void bmpDrawCallback(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h)
{
  // Serial.printf("Draw pos = %d, %d. size = %d x %d\n", x, y, w, h);
  gfx->draw16bitRGBBitmap(x, y, bitmap, w, h);
}

void draw_round_clock_mark(int16_t innerR1, int16_t outerR1, int16_t innerR2, int16_t outerR2, int16_t innerR3, int16_t outerR3)
{
  
  File bmpFile = SPIFFS.open("/NSRM12.bmp", "r");
  // read JPEG file header
    
    
    gfx->setCursor(50, 120);
    gfx->setTextSize(2 /* x scale */, 2 /* y scale */, 2);
    gfx->setTextColor(WHITE);
    gfx->println("Red Irish IPA");
    gfx->setCursor(70, 150);
    gfx->setTextSize(2 /* x scale */, 2 /* y scale */, 2);
    gfx->setTextColor(WHITE);
    gfx->println("IBU: 57");
    gfx->setCursor(70, 170);
    gfx->setTextSize(2 /* x scale */, 2 /* y scale */, 2);
    gfx->setTextColor(WHITE);
    gfx->println("ABV: 6.5%");
    gfx->setCursor(50, 190);
    gfx->setTextSize(2 /* x scale */, 2 /* y scale */, 2);
    gfx->setTextColor(WHITE);
    //Get Keg weight
    float kegWeight  = getWeight();
    int valueToPrint = (100/0.33) * kegWeight;
    gfx->println("Keg: " + String(kegWeight) + "Kg ");
    // Set the the position, gap between meters, and inner radius of the meters
    int xpos = 0, ypos = 5, gap = 4, radius = 120;

    // Draw meter and get back x position of next meter

    
    
    xpos = gap + ringMeter(valueToPrint, 0, 100, xpos, ypos, radius, "mA", RED2GREEN); // Draw analogue meter

    bmpClass.draw(
        &bmpFile, bmpDrawCallback, false /* useBigEndian */,
        0 /* x */, -50 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);

    bmpFile.close();
    
}

// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, char *units, uint8_t scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  x += r; y += r;   // Calculate coords of centre of ring

  int w = r / 8;    // Width of outer ring is 1/4 of radius
  
  int angle = 150;  // Half the sweep angle of meter (300 degrees)

  int text_colour = 5; // To hold the text colour

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 5; // Segments are 5 degrees wide = 60 segments for 300 degrees
  byte inc = 5; // Draw segments every 5 degrees, increase to 10 for segmented ring

  // Draw colour blocks every inc degrees
  for (int i = -angle; i < angle; i += inc) {

    // Choose colour from scheme
    int colour = 0;
    switch (scheme) {
      case 0: colour = 1; break; // Fixed colour
      case 1: colour = 2; break; // Fixed colour
      case 2: colour = 3; break; // Fixed colour
      case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
      case 4: colour = rainbow(map(i, -angle, angle, 63, 127)); break; // Green to red (high temperature etc)
      case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
      default: colour = 2; break; // Fixed colour
    }

    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      gfx->fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      gfx->fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      gfx->fillTriangle(x0, y0, x1, y1, x2, y2, 0);
      gfx->fillTriangle(x1, y1, x2, y2, x3, y3, 0);
    }
  }
}

void write_cache_pixel(int16_t x, int16_t y, int16_t color, bool cross_check_second, bool cross_check_hour)
{
    int16_t *cache = cached_points;
    if (cross_check_second)
    {
        for (uint16_t i = 0; i <= sHandLen; i++)
        {
            if ((x == *(cache++)) && (y == *(cache)))
            {
                return;
            }
            cache++;
        }
    }
    if (cross_check_hour)
    {
        cache = cached_points + ((sHandLen + 1) * 2);
        for (uint16_t i = 0; i <= hHandLen; i++)
        {
            if ((x == *(cache++)) && (y == *(cache)))
            {
                return;
            }
            cache++;
        }
    }
    gfx->writePixel(x, y, color);
}

void draw_and_erase_cached_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color, int16_t *cache, int16_t cache_len, bool cross_check_second, bool cross_check_hour)
{
       #if defined(ESP8266)
    yield();
#endif
    bool steep = _diff(y1, y0) > _diff(x1, x0);
    if (steep)
    {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    int16_t dx, dy;
    dx = _diff(x1, x0);
    dy = _diff(y1, y0);

    int16_t err = dx / 2;
    int8_t xstep = (x0 < x1) ? 1 : -1;
    int8_t ystep = (y0 < y1) ? 1 : -1;
    x1 += xstep;
    int16_t x, y, ox, oy;
    for (int16_t i = 0; i <= dx; i++)
    {
        if (steep)
        {
            x = y0;
            y = x0;
        }
        else
        {
            x = x0;
            y = y0;
        }
        ox = *(cache + (i * 2));
        oy = *(cache + (i * 2) + 1);
        if ((x == ox) && (y == oy))
        {
            if (cross_check_second || cross_check_hour)
            {
                write_cache_pixel(x, y, color, cross_check_second, cross_check_hour);
            }
        }
        else
        {
            write_cache_pixel(x, y, color, cross_check_second, cross_check_hour);
            if ((ox > 0) || (oy > 0))
            {
                write_cache_pixel(ox, oy, BACKGROUND, cross_check_second, cross_check_hour);
            }
            *(cache + (i * 2)) = x;
            *(cache + (i * 2) + 1) = y;
        }
        if (err < dy)
        {
            y0 += ystep;
            err += dx;
        }
        err -= dy;
        x0 += xstep;
    }
    for (int16_t i = dx + 1; i < cache_len; i++)
    {
        ox = *(cache + (i * 2));
        oy = *(cache + (i * 2) + 1);
        if ((ox > 0) || (oy > 0))
        {
            write_cache_pixel(ox, oy, BACKGROUND, cross_check_second, cross_check_hour);
        }
        *(cache + (i * 2)) = 0;
        *(cache + (i * 2) + 1) = 0;
    }
  
}



void redraw_hands_cached_draw_and_erase()
{
    gfx->startWrite();
    draw_and_erase_cached_line(center, center, nsx, nsy, SECOND_COLOR, cached_points, sHandLen + 1, false, false);
   // draw_and_erase_cached_line(center, center, nhx, nhy, HOUR_COLOR, cached_points + ((sHandLen + 1) * 2), hHandLen + 1, true, false);
   // draw_and_erase_cached_line(center, center, nmx, nmy, MINUTE_COLOR, cached_points + ((sHandLen + 1 + hHandLen + 1) * 2), mHandLen + 1, true, true);
    gfx->endWrite();
}

