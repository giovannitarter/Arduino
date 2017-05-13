#include "XPT2046_Touchscreen.h"
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>
#include "dht.h"

#define TFT_GREY 0x5AEB // New colour
#define CS_TOUCH 16

TFT_eSPI tft = TFT_eSPI();  // Invoke library
XPT2046_Touchscreen ts(CS_TOUCH);
#define DEG2RAD 0.0174532925


uint16_t x;
uint16_t y;
uint16_t z;

uint8_t temp_u = 0;
uint8_t temp_d = 0;
uint8_t hum_u = 0;
uint8_t hum_d = 0;

char str[40];

DHT dht = DHT(5, SENS_DHT12);


void setup() {
  
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_GREY);
  
  ts.begin();
  delay(200);
}


void loop() {
  
  // Set "cursor" at top left corner of display (0,0) and select font 2
  // (cursor will move to next line automatically during printing with 'tft.println'
  //  or stay on the line is there is room for the text with tft.print)
  tft.setCursor(0, 0, 2);
  
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE,TFT_GREY);  
  tft.setTextSize(1);
  
  // We can now plot text on screen using the "print" class


  if (ts.touched()) {
    ts.readData(&x, &y, &z);
    sprintf(str, "x:%d, y:%d, z:%d        ", x, y, z);
    Serial.println(str);
    tft.println(str);  
  }

  dht.read_dht();
  temp_u = dht.temp;
  temp_d = dht.temp_dec;
  hum_u = dht.hum;
  hum_d = dht.hum_dec;


   #define GAP 30
   tft.fillCircle(GAP, GAP, 5, TFT_BLUE);
   tft.fillCircle(GAP, 240 - GAP, 5, TFT_BLUE);
   tft.fillCircle(320 - GAP, 240 - GAP, 5, TFT_BLUE);
   tft.fillCircle(320 - GAP, GAP, 5, TFT_BLUE);


   #define TXT_SIZE 6
   int xpos = 30; 
   int ypos = 85;
   tft.setTextColor(TFT_RED, TFT_GREY);
   xpos += tft.drawNumber(temp_u, xpos, ypos, TXT_SIZE);
   xpos += tft.drawChar('.', xpos, ypos, TXT_SIZE); 
   xpos += tft.drawNumber(temp_d, xpos, ypos, TXT_SIZE);
   xpos += tft.drawChar('%', xpos, ypos, TXT_SIZE); 


   tft.setTextColor(TFT_BLUE, TFT_GREY); 
   xpos += 30;
   xpos += tft.drawNumber(hum_u, xpos, ypos, TXT_SIZE);
   xpos += tft.drawChar('.', xpos, ypos, TXT_SIZE); 
   xpos += tft.drawNumber(hum_d, xpos, ypos, TXT_SIZE);
   //xpos += tft.drawChar(0xdf, xpos, ypos, TXT_SIZE); 

      
   //xpos += tft.drawChar('', xpos, ypos, 8); 

//   fillArc(80, 180, 0, 15, 90, 90, 10, TFT_RED);

   //int tox = map(x, 515, 3400, GAP, 320-GAP);
   //int toy = map(y, 760, 3410, GAP, 240-GAP);

   //tft.drawCircle(tox, toy, 5, TFT_RED);

   //delay(2000);
  
}


// #########################################################################
// Draw a circular or elliptical arc with a defined thickness
// #########################################################################

// x,y == coords of centre of arc
// start_angle = 0 - 359
// seg_count = number of 6 degree segments to draw (60 => 360 degree arc)
// rx = x axis outer radius
// ry = y axis outer radius
// w  = width (thickness) of arc in pixels
// colour = 16 bit colour value
// Note if rx and ry are the same then an arc of a circle is drawn

int fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
{

  byte seg = 6; // Segments are 3 degrees wide = 120 segments for 360 degrees
  byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Calculate first pair of coordinates for segment start
  float sx = cos((start_angle - 90) * DEG2RAD);
  float sy = sin((start_angle - 90) * DEG2RAD);
  uint16_t x0 = sx * (rx - w) + x;
  uint16_t y0 = sy * (ry - w) + y;
  uint16_t x1 = sx * rx + x;
  uint16_t y1 = sy * ry + y;

  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * DEG2RAD);
    float sy2 = sin((i + seg - 90) * DEG2RAD);
    int x2 = sx2 * (rx - w) + x;
    int y2 = sy2 * (ry - w) + y;
    int x3 = sx2 * rx + x;
    int y3 = sy2 * ry + y;

    tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

    // Copy segment end to sgement start for next segment
    x0 = x2;
    y0 = y2;
    x1 = x3;
    y1 = y3;
  }
}

