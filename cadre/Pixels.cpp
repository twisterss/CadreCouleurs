#include "Pixels.h"

void Pixels::begin() {
    // Cut power
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, LOW);
  // Initialize the strip
  strip.begin(); 
  // Disconnect the data pin (start as off)
  pinMode(dataPin, INPUT);
  digitalWrite(dataPin, LOW);
}

void Pixels::switchOn() {
  if (!isOn) {
    // Power the strip
    digitalWrite(powerPin, HIGH);
    delay(1000);
    // Connect the data pin
    pinMode(dataPin, OUTPUT);
    digitalWrite(dataPin, LOW);
    isOn = true;
  }
}

void Pixels::switchOff() {
  if (isOn) {
    // Set the strip black
    clear();
    commit();
    delay(1000);
    // Disconnect the data pin
    pinMode(dataPin, INPUT);
    digitalWrite(dataPin, LOW);
    // Cut power
    digitalWrite(powerPin, LOW);
    isOn = false;
  }
}

void Pixels::set(uint8_t x, uint8_t y, uint32_t color) {
  strip.setPixelColor(getLedFromPixel(x, y), color);
}

uint32_t Pixels::get(uint8_t x, uint8_t y) {
  return strip.getPixelColor(getLedFromPixel(x, y));
}

void Pixels::commit() {
  switchOn();
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

uint32_t Pixels::getLedFromPixel(uint8_t x, uint8_t y) {
  return (PIXELS_Y - 1 - y) * PIXELS_X + (((PIXELS_Y - 1 - y) & 1) == 0 ? (PIXELS_X - 1 - x) : x);
}
