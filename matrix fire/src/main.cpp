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
DEFINE_GRADIENT_PALETTE(firePal_gp){
    0, 0, 0, 0,          // black
    100, 100, 0, 0,      // dark red
    128, 255, 0, 0,      // red
    220, 255, 255, 0,    // yellow
    255, 255, 255, 255}; // white
long previousTime;
void setBottomRow(uint16_t col);
void doFire();
void spreadFire(uint16_t src);
uint8_t firePixels[NUM_LEDS];
CRGBPalette16 _currentPalette = firePal_gp;

void setBottomRow(uint16_t col)
{
  for (uint16_t i = 0; i < kMatrixWidth; i++)
  {
    firePixels[(kMatrixHeight - 1) * kMatrixWidth + i] = col;
  }
}
void spreadFire(uint16_t src)
{
  if (firePixels[src] == 0)
  {
    firePixels[src - kMatrixWidth] = 0;
  }
  else
  {
    // Commented lines moves fire sideways as well as up, but doesn't look good on low res matrix:
    // int16_t dst = src - rand + 1;
    // firePixels[dst - kMatrixWidth] = firePixels[src] - random8(1);
    firePixels[src - kMatrixWidth] = firePixels[src] - random8(3);
  }
}
void doFire()
{
  for (uint16_t x = 0; x < kMatrixWidth; x++)
  {
    for (uint16_t y = 1; y < kMatrixHeight; y++)
    {
      spreadFire(y * kMatrixWidth + x);
    }
  }
}

void loop()
{
  setBottomRow(kMatrixHeight);
  doFire();
  for (int y = 0; y < kMatrixHeight; y++)
  {
    for (int x = 0; x < kMatrixWidth; x++)
    {
      int index = firePixels[kMatrixWidth * y + x];
      // Index goes from 0 -> kMatrixHeight, palette goes from 0 -> 255 so need to scale it
      uint8_t indexScale = 255 / kMatrixHeight;
      leds[XY(x, y)] = ColorFromPalette(_currentPalette, constrain(index * indexScale, 0, 255), 255, LINEARBLEND);
    }
  }

  FastLED.show();
}
