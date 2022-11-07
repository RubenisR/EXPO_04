#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN 3

#define COLOR_ORDER GRB
#define CHIPSET WS2812

#define BRIGHTNESS 155

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

void setup()
{
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);
}
uint16_t _plasmaShift = (random8(0, 5) * 32) + 64;
uint16_t _plasmaTime = 0;
const uint8_t _plasmaXfactor = 8;
const uint8_t _plasmaYfactor = 8;
void loop()
{

  // Fill background with dim plasma
  for (int16_t x = 0; x < kMatrixWidth; x++)
  {
    for (int16_t y = 0; y < kMatrixHeight; y++)
    {
      int16_t r = sin16(_plasmaTime) / 256;
      int16_t h = sin16(x * r * _plasmaXfactor + _plasmaTime) + cos16(y * (-r) * _plasmaYfactor + _plasmaTime) + sin16(y * x * (cos16(-_plasmaTime) / 256) / 2);
      leds[XY(x, y)] = CHSV((uint8_t)((h / 256) + 128), 255, 255);
    }
  }
  uint16_t oldPlasmaTime = _plasmaTime;
  _plasmaTime += _plasmaShift;
  if (oldPlasmaTime > _plasmaTime)
    _plasmaShift = (random8(0, 5) * 32) + 64;

  FastLED.show();
}
