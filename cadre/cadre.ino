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
#define MODE_RANDOM 4
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

/**
 * Return a random color different enough from the last one.
 * and not too dark or too bright.
 * All values rango from 0 to 255.
 */
uint32_t randomColor(uint8_t minLumi = 0, uint8_t maxLumi = 255, uint8_t minDist = 0) {
  static uint8_t oldRed = 0, oldGreen = 0, oldBlue = 0;
  uint8_t red, green, blue;
  uint16_t luminance = 0;
  uint16_t distance = 0;
  do {
    // Choose a color
    red = random(256);
    green = random(256);
    blue = random(256);
    // Compute luminance and distance from last color
    luminance = red;
    luminance+= red;
    luminance+= blue;
    luminance+= green;
    luminance+= green;
    luminance+= green;
    luminance/= 6;
    distance = max(max(abs(red - oldRed), abs(green - oldGreen)), abs(blue - oldBlue));
  } while (distance < minDist || luminance < minLumi || luminance > maxLumi);
  // Return the color
  oldRed = red;
  oldGreen = green;
  oldBlue = blue;
  return pixels.color(red, green, blue);
}

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

/**
 * Display random continually changing colors
 */
void displayGradient(bool newDisplay) {
  static uint32_t color;
  static uint8_t remaining = 0;
  if (newDisplay)
    pixels.clear();
  if (newDisplay || remaining == 0) {
    remaining = pixels.width() * pixels.height();
    color = randomColor(20, 200, 100);
  }
  if (skipSteps(300)) {
    // Find a pixel to change
    uint8_t x, y;
    do {
      x = random(pixels.width());
      y = random(pixels.height());
    }
    while (pixels.get(x, y) == color);
    pixels.set(x, y, color);
    pixels.commit();
    remaining--;
  }
}

/**
 * Display some geometric shapes
 */
void displayRandom(bool newDisplay) {
  if (skipSteps(5)) {
    pixels.set(random(pixels.width()), random(pixels.height()), randomColor());
    pixels.commit();
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
    case MODE_RANDOM:
      displayRandom(newOrderReceived);
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
    if (currentMode == MODE_CLOCK || currentMode == MODE_COLOR || currentMode == MODE_TEXT)
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
