/*
 * Represents the pixels display
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define PIXELS_X 12
#define PIXELS_Y 10

class Pixels {
   
  public:
    // LED strip
    Adafruit_NeoPixel strip;
    
    /**
     * Pixels constructor
     */
    Pixels(uint8_t pin):
      strip(PIXELS_X*PIXELS_Y, pin, NEO_GRB + NEO_KHZ800)
    {
    }
    
    /**
     * Begin controlling the pixels
     */
    void begin();

    /**
     * Set the color of a pixel in local memory
     */
    void set(uint8_t x, uint8_t y, uint32_t color, uint8_t invertX = 0, uint8_t invertY = 0);

    /**
     * Get the color of a pixel in local memory
     */
    uint32_t get(uint8_t x, uint8_t y, uint8_t invertX = 0, uint8_t invertY = 0);

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

  private:

    /**
     * Get the LED number from a pixel location
     */
    uint32_t getLedFromPixel(uint8_t x, uint8_t y, uint8_t invertX = 0, uint8_t invertY = 0);
};

