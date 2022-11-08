#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN 3

#define COLOR_ORDER GRB
#define CHIPSET WS2811

#define BRIGHTNESS 64
unsigned long time = 0;
long previousTime = 0;
uint16_t _plasmaShift = (random8(0, 5) * 32) + 64;
uint16_t _plasmaTime = 0;
const uint8_t _plasmaXfactor = 8;
const uint8_t _plasmaYfactor = 8;

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
void drawCircle(int xc, int yc, int r, CRGB color)
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
}
void setup()
{
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);
}
void drawPattern(uint8_t pattern)
{
  drawCircle(kMatrixWidth / 2, kMatrixHeight / 2, pattern, CHSV(hue, 255, 255));
}
void circle()
{
  void drawCircle(int xc, int yc, int r, CRGB color);
  void drawPattern(uint8_t pattern);
  fadeToBlackBy(leds, NUM_LEDS, 20);
  EVERY_N_MILLISECONDS(50)
  {
    if (_pattern == 8)
      hue += 32;
    _pattern = (_pattern + 1) % 9;
    drawPattern(_pattern);
  }
  FastLED.show();
}
void matrixmovie()
{
  if (millis() - previousTime >= 75)
  {
    // Move bright spots downward
    for (int row = kMatrixHeight - 1; row >= 0; row--)
    {
      for (int col = 0; col < kMatrixWidth; col++)
      {
        if (leds[XY(col, row)] == CRGB(175, 255, 175))
        {
          leds[XY(col, row)] = CRGB(0, 235, 0); // create trail
          if (row < kMatrixHeight - 1)
            leds[XY(col, row + 1)] = CRGB(175, 255, 175);
        }
      }
    }

    // Fade all leds
    for (int i = 0; i < NUM_LEDS; i++)
    {
      if (leds[i].g != 255)
        leds[i].nscale8(192); // only fade trail
    }

    // Spawn new falling spots
    if (random8(2) == 0) // lower number == more frequent spawns
    {
      int8_t spawnX = random8(kMatrixWidth);
      leds[XY(spawnX, 0)] = CRGB(175, 255, 175);
    }

    FastLED.show();
    previousTime = millis();
  }
}

void plasma()
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
long previousTime2 = 0;
void crosshatch()
{
  // Fade deals with 'tails'
  fadeToBlackBy(leds, NUM_LEDS, 5);

  if (millis() - previousTime2 >= 50)
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

    previousTime2 = millis();
  }

  FastLED.show();
}
DEFINE_GRADIENT_PALETTE(firePal_gp){
    0, 0, 0, 0,          // black
    100, 100, 0, 0,      // dark red
    128, 255, 0, 0,      // red
    220, 255, 255, 0,    // yellow
    255, 255, 255, 255}; // white
long previousTime3;
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
void fire()
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
void loop()
{
  time = millis();
  if (time <= 60000)
  {
    circle();
  }
  else if (time >= 60000 && time <= 120000)
  {
    matrixmovie();
  }
  else if (time >= 120000 && time <= 180000)
  {
    plasma();
  }
  else if (time >= 180000 && time <= 240000)
  {
    crosshatch();
  }
  else if (time >= 240000 && time <= 300000)
  {
    fire();
  }
  else if (time >= 300000)
  {
    time = 0;
  }
}
