#include "config.h"
#include "display.h"
#include "weightmeasure.h"
#include <FS.h>
#include <SPIFFS.h>
#include <Arduino_GFX_Library.h>
#include "BmpClass.h"
#include <ESPAsyncWebServer.h>
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
int keg_level = 0;
#include "BmpClass.h"
#include <WString.h>
#include <String.h>
#include "Preferences.h"
static BmpClass bmpClass;

// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

//Preferences preferences;

// pixel drawing callback
static void bmpDrawCallback(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h)
{
  // Serial.printf("Draw pos = %d, %d. size = %d x %d\n", x, y, w, h);
  gfx->draw16bitRGBBitmap(x, y, bitmap, w, h);
}


void display_init()
{
     gfx->begin();
      Serial.println("Display begin");
    gfx->fillScreen(BACKGROUND);
    //preferences.begin("TapDisplay", true);
  
#ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
#endif
    gfx->setCursor(50, 120);
    gfx->setTextSize(2 /* x scale */, 2 /* y scale */, 2);
    gfx->setTextColor(WHITE);

    String beer_name = getBeerName();
    String beer_IBU = (String)getBeerIBU();
    String beer_Sg = (String)getBeerSg();
    String beer_ABV = (String)getBeerABV();
    String beer_File = getFileForEBC();
    int beer_EBC = getBeerEBC();
     Serial.println("Load file");
    File bmpFile = SPIFFS.open("/"+ beer_File, "r");
    // read JPEG file header
     Serial.println("File loaded");
    Serial.println("Beer Name: " + beer_name);
    Serial.println("Beer IBU: " + beer_IBU);
    Serial.println("Beer Sg: " + beer_Sg);
     Serial.println("Beer ABV: " + beer_ABV);
     Serial.println("Beer File: " + beer_File);
    gfx->setCursor(50, 120);
    gfx->setTextSize(2 /* x scale */, 2 /* y scale */, 2);
    gfx->setTextColor(WHITE);
    gfx->println(beer_name);
    Serial.println(beer_name);
    gfx->setCursor(70, 150);
    gfx->setTextSize(2 /* x scale */, 2 /* y scale */, 2);
    gfx->setTextColor(WHITE);
    gfx->println("IBU: " + beer_IBU);
    gfx->setCursor(70, 170);
    gfx->setTextSize(2 /* x scale */, 2 /* y scale */, 2);
    gfx->setTextColor(WHITE);
    gfx->println("ABV: " + beer_ABV);
    gfx->setCursor(50, 190);
     Serial.println("DrawBMP");
    bmpClass.draw(
        &bmpFile, bmpDrawCallback, false /* useBigEndian */,
        0 /* x */, -50 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);

    bmpFile.close();
    Serial.println("End of display init");
}

void refresh_display()
{
   // File dataFile = SPIFFS.open("/param.xml", "r");
    //gfx->fillScreen(BACKGROUND);
    // init LCD constant
    clearDynamicZones();
   updateMeter();

    // Draw 60 clock marks
   // draw_round_clock_mark(
    // draw_square_clock_mark(
      //  center - markLen, center,
       // center - (markLen * 2 / 3), center,
       // center - (markLen / 2), center);
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



void clearDynamicZones()
{
    //gfx->fillRect(50,190,230,20,BLACK);
    //clearMeter(100)
}



void updateMeter()
{
  
    
    //Get Keg weight
     Serial.println("Get weight");
    //float kegWeight  = getWeight();
    // Serial.println("Weight:");
     float volume = getVolume();
    int valueToPrint = (100/19) * volume;
   // gfx->setCursor(50, 190);
    //gfx->setTextSize(2 /* x scale */, 2 /* y scale */, 2);
    //gfx->setTextColor(BLACK);
    //gfx->println("Keg: " + String(volume) + "L");
    //gfx->setCursor(50, 190);
    //gfx->setTextSize(2 /* x scale */, 2 /* y scale */, 2);
    //gfx->setTextColor(WHITE);
    //gfx->println("Keg: " + String(volume) + "L");
    // Set the the position, gap between meters, and inner radius of the meters
    int xpos = 0, ypos = 5, gap = 4, radius = 120;

    // Draw meter and get back x position of next meter
    xpos = gap + ringMeter(valueToPrint, 0, 100, xpos, ypos, radius, "L", RED2GREEN); // Draw analogue meter
   
    //preference.end();
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




