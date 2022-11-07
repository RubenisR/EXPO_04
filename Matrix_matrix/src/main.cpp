#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN 3

#define COLOR_ORDER GRB
#define CHIPSET WS2812

#define BRIGHTNESS 255

// Params for width and height
const uint8_t kMatrixWidth = 8;
const uint8_t kMatrixHeight = 8;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds[NUM_LEDS];
#define LAST_VISIBLE_LED 45
uint8_t XY(uint8_t x, uint8_t y)
{
  // any out of bounds address maps to the first hidden pixel
  if ((x >= kMatrixWidth) || (y >= kMatrixHeight))
  {
    return (LAST_VISIBLE_LED + 1);
  }

  const uint8_t XYTable[] = {
      0, 8, 46, 51, 55, 59, 30, 38,
      1, 9, 16, 52, 56, 27, 31, 39,
      2, 10, 17, 19, 23, 28, 32, 40,
      3, 11, 18, 20, 24, 29, 33, 41,
      4, 12, 47, 21, 25, 60, 34, 42,
      5, 13, 48, 22, 26, 61, 35, 43,
      6, 14, 49, 53, 57, 62, 36, 44,
      7, 15, 50, 54, 58, 63, 37, 45};

  uint8_t i = (y * kMatrixWidth) + x;
  uint8_t j = XYTable[i];
  return j;
}
uint8_t _pattern = 0;
uint8_t hue = 0;
/* void drawCircle(int xc, int yc, int r, CRGB color)
{
  int x = -r;
  int y = 0;
  int e = 2 - (2 * r);
  do
  {
    leds[XY(xc + x, yc - y)] = color;
    leds[XY(xc - x, yc + y)] = color;
    leds[XY(xc + y, yc + x)] = color;
    leds[XY(xc - y, yc - x)] = color;
    int _e = e;
    if (_e <= y)
      e += (++y * 2) + 1;
    if ((_e > x) || (e > y))
      e += (++x * 2) + 1;
  } while (x < 0);
}*/

/*void drawPattern(uint8_t pattern)
{
  drawCircle(kMatrixWidth / 2, kMatrixHeight / 2, pattern, CHSV(hue, 255, 255));
}*/

void setup()
{
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);
}
long previousTime = 0;
void loop()
{
  if(millis() - previousTime >= 75) {
    // Move bright spots downward
    for (int row = kMatrixHeight - 1; row >= 0; row--) {
      for (int col = 0; col < kMatrixWidth; col++) {
        if (leds[XY(col, row)] == CRGB(175,255,175)) {
          leds[XY(col, row)] = CRGB(0,235,0); // create trail
          if (row < kMatrixHeight - 1) leds[XY(col, row + 1)] = CRGB(175,255,175);
        }
      }
    }
    
    // Fade all leds
    for(int i = 0; i < NUM_LEDS; i++) {
      if (leds[i].g != 255) leds[i].nscale8(192); // only fade trail
    }

    // Spawn new falling spots
    if (random8(2) == 0) // lower number == more frequent spawns
    {
      int8_t spawnX = random8(kMatrixWidth);
      leds[XY(spawnX, 0)] = CRGB(175,255,175 );
    }

    FastLED.show();
    previousTime = millis();
  }
}
