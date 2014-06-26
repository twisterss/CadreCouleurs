/*
 * Represents the pixels display
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define PIXELS_X 12
#define PIXELS_Y 10

class Pixels {

  private:

    // LED strip
    Adafruit_NeoPixel strip;

    /**
     * Get the LED number from a pixel location
     */
    uint32_t getLedFromPixel(uint8_t x, uint8_t y);

    /**
     * Pin to send data to the strip
     */
    uint8_t dataPin;

    /**
     * Pin to switch the strip on and off
     */
    uint8_t powerPin;

    /**
     * Is the strip currently on?
     */
    bool isOn;
   
  public:
    
    /**
     * Pixels constructor
     */
    Pixels(uint8_t dataPin, uint8_t powerPin):
      strip(PIXELS_X*PIXELS_Y, dataPin, NEO_GRB + NEO_KHZ800),
      dataPin(dataPin),
      powerPin(powerPin),
      isOn(false)
    {
    }
    
    /**
     * Begin controlling the pixels
     */
    void begin();

    /**
     * Switch the strip ON if not already.
     */
    void switchOn();

    /**
     * Switch the strip OFF if not already.
     * Cut the power to the strip
     */
    void switchOff();

    /**
     * Set the color of a pixel in local memory
     */
    void set(uint8_t x, uint8_t y, uint32_t color);

    /**
     * Get the color of a pixel in local memory
     */
    uint32_t get(uint8_t x, uint8_t y);

    /**
     * Display what is in local memory
     */
    void commit();

    /**
     * Set all pixels to black
     */
    void clear(uint32_t color = 0);

    /**
     * Get a color properly coded
     */
    uint32_t color(uint8_t red, uint8_t green, uint8_t blue);

    /**
     * Get the width (in pixels)
     */
    uint8_t width();

    /**
     * Get the height (in pixels)
     */
    uint8_t height();
};

