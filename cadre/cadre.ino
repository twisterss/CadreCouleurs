#include <SPI.h>
#include <Ethernet.h>
#include <Time.h> 
#include <Adafruit_NeoPixel.h>
#include "Pixels.h"
#include "WebServer.h"

// Hardware configuration
#define STRIP_PIN 6

// Minimum delay between two display states (microseconds)
#define DISPLAY_DELAY 1000
#define MAX_TEXT_SIZE 30

// Working modes
#define MODE_OFF 0
#define MODE_CLOCK 1
#define MODE_COLOR 2
#define MODE_GRADIENT 3
#define MODE_GEO 4
#define MODE_TEXT 5
#define MODE_INIT 255

// MAC address of the server
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x00, 0xEA, 0xF8
};

// Letters to display (5x5)
const byte letters[38][5] = {
  {0x7c,0x44,0x44,0x7c,0x44}, // A
  {0x7c,0x44,0x78,0x44,0x7c},  
  {0x7c,0x40,0x40,0x40,0x7c},  
  {0x78,0x44,0x44,0x44,0x78},  
  {0x7c,0x40,0x78,0x40,0x7c},
  {0x7c,0x40,0x70,0x40,0x40},
  {0x7c,0x40,0x4c,0x44,0x7c},
  {0x44,0x44,0x7c,0x44,0x44},
  {0x7c,0x10,0x10,0x10,0x7c},
  {0x0c,0x04,0x04,0x44,0x7c},
  {0x44,0x48,0x70,0x48,0x44},
  {0x40,0x40,0x40,0x40,0x7c},
  {0x44,0x6c,0x54,0x44,0x44},
  {0x44,0x64,0x54,0x4c,0x44},
  {0x38,0x44,0x44,0x44,0x38},  
  {0x78,0x44,0x78,0x40,0x40},              
  {0x7c,0x44,0x44,0x7c,0x10},            
  {0x78,0x44,0x78,0x44,0x44},            
  {0x7c,0x40,0x7c,0x04,0x7c},            
  {0x7c,0x10,0x10,0x10,0x10},              
  {0x44,0x44,0x44,0x44,0x7c},            
  {0x44,0x44,0x28,0x28,0x10},            
  {0x44,0x44,0x54,0x54,0x28},            
  {0x44,0x28,0x10,0x28,0x44},            
  {0x44,0x44,0x28,0x10,0x10},            
  {0x7c,0x08,0x10,0x20,0x7c}, // Z
  {0x38,0x4C,0x54,0x64,0x38}, // 0
  {0x70,0x10,0x10,0x10,0x7C},
  {0x7C,0x04,0x7C,0x40,0x7C},
  {0x7C,0x04,0x1C,0x04,0x7C},
  {0x40,0x48,0x7C,0x08,0x08},
  {0x7C,0x40,0x7C,0x04,0x7C},
  {0x7C,0x40,0x7C,0x44,0x7C},
  {0x78,0x08,0x1C,0x08,0x08},
  {0x38,0x44,0x38,0x44,0x38},
  {0x7C,0x44,0x7C,0x04,0x7C}, // 9  
  {0x00,0x00,0x00,0x00,0x00}, // space
  {0x00,0x00,0x00,0x30,0x30}  // dot
};

// Web server
WebServer server(80);
// LED strip
Pixels pixels(STRIP_PIN);

// Display state
unsigned long lastDisplayEvent = 0;
uint8_t currentMode = MODE_INIT;
bool newOrderReceived = true;
// Current configuration (used by all modes)
uint32_t color1 = pixels.color(255, 255, 255), color2 = pixels.color(0, 0, 0);
char text[MAX_TEXT_SIZE];

void fixedText(const char* string, uint32_t color, int16_t startX = 0, int16_t startY = 2) {
  uint16_t length = strlen(string);
  uint16_t letterIndex = 0;
  uint8_t letter = 0;
  uint8_t column = 0;
  for (int16_t x = startX; x < pixels.width(); x++) {
    if (column == 0) {
      // Select the letter
      // Finished writing the text?
      if (letterIndex >= length)
        return;
      letter = string[letterIndex];
      // Convert ASCII to table index
      if (letter >= 65 && letter <= 90)
        letter-= 65;
      else if (letter >= 97 && letter <= 122)
        letter-= 97;
      else if (letter >= 48 && letter <= 57)
        letter-= 22;
      else if (letter == 46)
        letter = 37;
      else
        letter = 36;
      letterIndex++;
    }
    for (int16_t y = startY; y < pixels.height() && y < startY + 5; y++) {
      if (x >= 0 && x < pixels.width() && y >= 0 && y < pixels.height() && (letters[letter][y - startY] >> (6 - column)) & 1)
        pixels.set(x, y, color);
    }
    column++;
    if (column >= 6)
      column = 0;
  }
}

/**
 * Utility to so sometheing every n steps.
 * Returns true every n steps
 */
bool skipSteps(uint16_t steps) {
  static uint16_t step = 0;
  step++;
  if (step >= steps) {
    step = 0;
    return true;
  }
  return false;
}

void displayInit(bool newDisplay) {
  static uint16_t startCounter = 0;
  // Display the IP address at the start
  if (newDisplay) {
    String address = String(server.localIP()[0]) + "." + String(server.localIP()[1]) + "." + String(server.localIP()[2]) + "." + String(server.localIP()[3]);
    address.toCharArray(text, MAX_TEXT_SIZE);
    startCounter = 0;
  }
  // End the initialization after some time
  if (startCounter >= 30000) {
    displayGradient(true);
    currentMode = MODE_GRADIENT;
  } else {
    displayText(newDisplay);
    startCounter++;
  }
}

/**
 * Display nothing
 */
void displayOff(bool newDisplay) {
  if (newDisplay) {
    pixels.clear();
    pixels.commit();
  }
}

/**
 * Display the time
 */
void displayClock(bool newDisplay) {
  if (newDisplay || skipSteps(1000)) {
    pixels.clear();
    char tmpChars[3];
    String tmpString;
    tmpString = String(hour());
    if (tmpString.length() < 2)
      tmpString = "0" + tmpString;
    tmpString.toCharArray(tmpChars, 3);
    fixedText(tmpChars, color1, 1, 0);
    tmpString = String(minute());
    if (tmpString.length() < 2)
      tmpString = "0" + tmpString;
    tmpString.toCharArray(tmpChars, 3);
    fixedText(tmpChars, color2, 1, 5);
    pixels.commit();
  }
}

/**
 * Display a constant color on the frame
 */
void displayConstantColor(bool newDisplay) {
  static int y = 0;
  if (newDisplay)
    y = 0;
  if (skipSteps(50)) {
    if (y < pixels.height()) {
      for (uint8_t x = 0; x < pixels.width(); x++)
          pixels.set(x, y, color1);
      pixels.commit();
      y++;
    }
  }
}

void displayGradient(bool newDisplay) {
  static uint32_t oldColor, newColor;
  static uint8_t remaining = 0;
  if (newDisplay)
    pixels.clear();
  if (newDisplay || remaining == 0) {
    remaining = pixels.width() * pixels.height();
    oldColor = newColor;
    uint16_t luminance = 0;
    while (oldColor == newColor || luminance < 30 || luminance > 225) {
      // Choose a bright enough random color
      uint8_t red, green, blue;
      red = random(256);
      green = random(256);
      blue = random(256);
      newColor = pixels.color(red, green, blue);
      luminance = red;
      luminance+= red;
      luminance+= blue;
      luminance+= green;
      luminance+= green;
      luminance+= green;
      luminance/= 6;
    }
  }
  if (skipSteps(300)) {
    // Find a pixel to change
    uint8_t x, y;
    do {
      x = random(pixels.width());
      y = random(pixels.height());
    }
    while (pixels.get(x, y) == newColor);
    pixels.set(x, y, newColor);
    pixels.commit();
    remaining--;
  }
}

/**
 * Display some geometric shapes
 */
void displayGeo(bool newDisplay) {
  static uint8_t step = 0;
  static uint8_t curX = 0, curY = 0;
  static bool inverted = 0;
  if (newDisplay) {
    step = 0;
    inverted = 0;
  }
  if (skipSteps(5)) {
    if (step == pixels.width() * pixels.height()) {
      step = 0;
      inverted = !inverted;
    }
    if (step % pixels.height() == 0) {
      curX = step / pixels.height();
      for (uint8_t y = 0; y < pixels.height(); y++) {
        if (curX > 0 && y != curY)
          pixels.set(curX-1, y, pixels.color(0, 0, 0), inverted, inverted);
        pixels.set(curX, y, color1, inverted, inverted);
      }
    }
    if (step % pixels.width() == 0) {
      curY = step / pixels.width();
      for (uint8_t x = 0; x < pixels.width(); x++) {
        if (curY > 0 && x != curX)
          pixels.set(x, curY-1, pixels.color(0, 0, 0), inverted, inverted);
        pixels.set(x, curY, color1, inverted, inverted);
      }
    }
    pixels.commit();
    step++;
  }
}

/**
 * Display a sliding text
 */
void displayText(bool newDisplay) {
  static uint16_t columns = 0;
  static uint16_t column = 0;
  if (newDisplay || column == columns) {
    columns = strlen(text) * 6 + pixels.width();
    column = 0;
  }
  if (skipSteps(100)) {
    pixels.clear(color2);
    fixedText(text, color1, pixels.width() - 1 - column);
    pixels.commit();
    column++;
  }
}

/**
 * Manage one generic display step
 */
void display() {
  switch(currentMode) {
    case MODE_INIT:
      displayInit(newOrderReceived);
      break;
    case MODE_OFF:
      displayOff(newOrderReceived);
      break;
    case MODE_CLOCK:
      displayClock(newOrderReceived);
      break;
    case MODE_COLOR:
      displayConstantColor(newOrderReceived);
      break;
    case MODE_GRADIENT:
      displayGradient(newOrderReceived);
      break;
    case MODE_GEO:
      displayGeo(newOrderReceived);
      break;
    case MODE_TEXT:
      displayText(newOrderReceived);
      break;
  }
  newOrderReceived = false;
}

/**
 * Dynamic response to server requests
 */
WebResponse listenToRequests(WebRequest &request) {
  if (request.resource == "setMode") {
    // Received a request to change the current mode
    // Decode a bit
    request.params.replace("%20", " ");
    // Get the mode
    currentMode = request.params.substring(0, 2).toInt();
    // Get the settings
    if (currentMode == MODE_CLOCK || currentMode == MODE_COLOR || currentMode == MODE_TEXT || currentMode == MODE_GEO)
      color1 = pixels.color(request.params.substring(2, 5).toInt(), request.params.substring(5, 8).toInt(), request.params.substring(8, 11).toInt());
    if (currentMode == MODE_CLOCK || currentMode == MODE_TEXT)
      color2 = pixels.color(request.params.substring(11, 14).toInt(), request.params.substring(14, 17).toInt(), request.params.substring(17, 20).toInt());
    if (currentMode == MODE_TEXT)
      request.params.substring(20).toCharArray(text, MAX_TEXT_SIZE);
    if (currentMode == MODE_CLOCK)
      setTime(request.params.substring(20).toInt());
    newOrderReceived = true;
    // Send an OK response
    WebResponse response = WebResponse::createText("OK");
    response.setAllowAllOrigins();
    return response;
  }
  return WebResponse::createNull();
}

/**
 * Initialization procedure
 */
void setup() {
  // Random generator initialization
  randomSeed(analogRead(0));
  // Network initialization
  server.registerServeMethod(listenToRequests);
  server.begin(mac);
  // Pixels initialization
  pixels.begin();
  pixels.commit(); // Initialize all pixels to 'off'
}

/**
 * Main ever-running procedure
 */
void loop() {
  // Serve one HTTP request if any
  server.serve();
  // Display periodically
  static unsigned long lastDisplayEvent = 0;
  const unsigned long now = micros();
  const unsigned long next = lastDisplayEvent + DISPLAY_DELAY;
  if (now >= next) {
    display();
    lastDisplayEvent = now;
  }
}
