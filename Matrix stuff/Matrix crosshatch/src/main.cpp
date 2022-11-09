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
struct Blobs
{
  uint8_t _x;
  uint8_t _y;
  bool _right;
  CHSV _colour;
};
Blobs blob[20];
uint8_t blobCounter = 0;
const uint8_t blobRate = 7; // Higher number is fewer blobs
long previousTime = 0;
void loop()
{

  // Fade deals with 'tails'
  fadeToBlackBy(leds, NUM_LEDS, 5);

  if (millis() - previousTime >= 50)
  {
    // Spawn new horizontal blob
    if (random8(blobRate) == 0)
    {
      uint8_t spawnY = random8(kMatrixHeight);
      blob[blobCounter] = {0, spawnY, true, CHSV(random8(), 255, 255)};
      blobCounter = (blobCounter + 1) % 20;
    }

    // Spawn new vertical blob
    if (random8(blobRate) == 0)
    {
      uint8_t spawnX = random8(kMatrixWidth);
      blob[blobCounter] = {spawnX, 0, false, CHSV(random8(), 255, 255)};
      blobCounter = (blobCounter + 1) % 20;
    }

    // Draw the blobs
    for (int i = 0; i < 20; i++)
    {
      leds[XY(blob[i]._x, blob[i]._y)] = leds[XY(blob[i]._x, blob[i]._y)] + blob[i]._colour;
    }

    // Move the blobs
    for (int i = 0; i < 20; i++)
    {
      if (blob[i]._right)
      {
        blob[i]._x++;
      }
      else
      {
        blob[i]._y++;
      }
    }

    previousTime = millis();
  }

  FastLED.show();
}
