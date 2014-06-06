#include "Pixels.h"

void Pixels::begin() {
  strip.begin(); 
}

void Pixels::set(uint8_t x, uint8_t y, uint32_t color, uint8_t invertX, uint8_t invertY) {
  strip.setPixelColor(getLedFromPixel(x, y, invertX, invertY), color);
}

uint32_t Pixels::get(uint8_t x, uint8_t y, uint8_t invertX, uint8_t invertY) {
  return strip.getPixelColor(getLedFromPixel(x, y, invertX, invertY));
}

void Pixels::commit() {
  strip.show();
}

void Pixels::clear(uint32_t color) {
  for (uint8_t x = 0; x < PIXELS_X; x++) 
    for (uint8_t y = 0; y < PIXELS_Y; y++) 
      set(x, y, color);
}

uint32_t Pixels::color(uint8_t red, uint8_t green, uint8_t blue) {
  return strip.Color(red, green, blue);
}

uint8_t Pixels::width() {
  return PIXELS_X;
}

uint8_t Pixels::height() {
  return PIXELS_Y;
}

uint32_t Pixels::getLedFromPixel(uint8_t x, uint8_t y, uint8_t invertX, uint8_t invertY) {
  if (invertX)
    x = PIXELS_X - 1 - x;
  if (invertY)
    y = PIXELS_Y - 1 - y;
  return (PIXELS_Y - 1 - y) * PIXELS_X + (((PIXELS_Y - 1 - y) & 1) == 0 ? (PIXELS_X - 1 - x) : x);
}
