#include <avr/wdt.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>  
#include "RTCControl.h"
#include <Time.h> 
#include <Adafruit_NeoPixel.h>
#include "Pixels.h"
#include "WebServer.h"

// Hardware configuration
#define STRIP_PIN_DATA 6
#define STRIP_PIN_POWER A3
#define BUTTON_PIN 7

// Minimum delay between two display states (microseconds)
#define DISPLAY_DELAY 1000
// Maximum number of characters for a displayed message
#define MAX_TEXT_SIZE 30

// Working modes
#define MODE_OFF 0
#define MODE_CLOCK 1
#define MODE_COLOR 2
#define MODE_GRADIENT 3
#define MODE_RANDOM 4
#define MODE_TEXT 5
#define MODE_DEMO 6
#define MODE_ADDRESS 255

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
Pixels pixels(STRIP_PIN_DATA, STRIP_PIN_POWER);

// Display state
unsigned long lastDisplayEvent = 0;
bool newOrderReceived = true;
bool newOrderInternal = false;

// Current configuration (used by all modes)
typedef struct State {
  uint8_t mode;
  uint32_t color1;
  uint32_t color2;
  char text[MAX_TEXT_SIZE];
} State;
State current;

/**
 * Load the current state from RTC RAM
 */
void loadCurrent() {
  RTC.readBytesInRam(0, sizeof(current), (uint8_t*) &current);
}

/**
 * Save the current state in RTC RAM
 */
void saveCurrent() {
  RTC.writeBytesInRam(0, sizeof(current), (uint8_t*) &current);
}

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
    distance = (abs(red - oldRed) + abs(green - oldGreen) + abs(blue - oldBlue)) / 3;
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

/**
 * Display the IP address of the frame once
 */
void displayAddress(bool newDisplay) {
  if (newDisplay) {
    String address = String(server.localIP()[0]) + "." + String(server.localIP()[1]) + "." + String(server.localIP()[2]) + "." + String(server.localIP()[3]);
    address.toCharArray(current.text, MAX_TEXT_SIZE);
    current.color1 = pixels.color(255, 255, 255);
    current.color2 = pixels.color(0, 0, 0);
  }
  if (displayText(newDisplay)) {
    // IP displayed: back to last mode
    newOrderInternal = true;
    loadCurrent();
  }
}

/**
 * Display nothing
 */
void displayOff(bool newDisplay) {
  if (newDisplay) {
    pixels.switchOff();
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
    fixedText(tmpChars, current.color1, 0, 0);
    tmpString = String(minute());
    if (tmpString.length() < 2)
      tmpString = "0" + tmpString;
    tmpString.toCharArray(tmpChars, 3);
    fixedText(tmpChars, current.color2, 1, 5);
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
          pixels.set(x, y, current.color1);
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
    color = randomColor(20, 200, 60);
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
 * Display a sliding text.
 * Returns if it is an end of text
 */
bool displayText(bool newDisplay) {
  static uint16_t columns = 0;
  static uint16_t column = 0;
  if (newDisplay || column == columns) {
    columns = strlen(current.text) * 6 + pixels.width();
    column = 0;
  }
  if (skipSteps(100)) {
    pixels.clear(current.color2);
    fixedText(current.text, current.color1, pixels.width() - 1 - column);
    pixels.commit();
    column++;
  }
  return column == columns;
}

/**
 * Display a bit of everything for a demo
 */
void displayDemo(bool newDisplay) {
  static uint8_t mode;
  static uint16_t duration;
  bool newMode = false;
  if (newDisplay) {
    mode = MODE_RANDOM;
    duration = 0;
    newMode = true;
  }
  if (duration >= 30000) {
    // Change the mode
    if (mode == MODE_RANDOM) {
      mode = MODE_TEXT;
      strcpy(current.text, "Cadre couleurs");
      current.color1 = pixels.color(255, 255, 255);
      current.color2 = pixels.color(0, 0, 0);
    } else if (mode == MODE_TEXT) {
      mode = MODE_GRADIENT;
    } else if (mode == MODE_GRADIENT) {
      mode = MODE_CLOCK;
      current.color1 = pixels.color(180, 0, 0);
      current.color2 = pixels.color(100, 100, 0);
    } else {
      mode = MODE_RANDOM;
    }
    newMode = true;
    duration = 0;
  }
  switch(mode) {
    case MODE_CLOCK:
      displayClock(newMode);
      break;
    case MODE_GRADIENT:
      displayGradient(newMode);
      break;
    case MODE_RANDOM:
      displayRandom(newMode);
      break;
    case MODE_TEXT:
      displayText(newMode);
      break;
  }
  duration++;
}

/**
 * Manage one generic display step
 */
void display() {
  switch(current.mode) {
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
    case MODE_DEMO:
      displayDemo(newOrderReceived);
      break;
    case MODE_ADDRESS:
      displayAddress(newOrderReceived);
      break;
  }
  newOrderReceived = newOrderInternal;
  newOrderInternal = false;
}

/**
 * Dynamic response to server requests
 */
WebResponse listenToRequests(WebRequest &request) {
  if (request.resource == "setMode") {
    // Received a request to change the current mode
    // Decode url-encoding
    request.params.replace("%20", " ");
    // Get the mode
    current.mode = request.params.substring(0, 2).toInt();
    // Get the settings
    if (current.mode == MODE_CLOCK || current.mode == MODE_COLOR || current.mode == MODE_TEXT)
      current.color1 = pixels.color(request.params.substring(2, 5).toInt(), request.params.substring(5, 8).toInt(), request.params.substring(8, 11).toInt());
    if (current.mode == MODE_CLOCK || current.mode == MODE_TEXT)
      current.color2 = pixels.color(request.params.substring(11, 14).toInt(), request.params.substring(14, 17).toInt(), request.params.substring(17, 20).toInt());
    if (current.mode == MODE_TEXT)
      request.params.substring(20).toCharArray(current.text, MAX_TEXT_SIZE);
    if (current.mode == MODE_CLOCK) {
      // Set the current time
      time_t time = request.params.substring(20).toInt();
      RTC.set(time);
      setTime(time);
    }
    // Save the state in RTC RAM
    saveCurrent();
    newOrderReceived = true;
    // Send an OK response
    WebResponse response = WebResponse::createText("OK");
    response.setAllowAllOrigins();
    return response;
  }
  return WebResponse::createNull();
}

/**
 * Check the current state of the button.
 * Used to display the IP address
 */
void checkButton() {
  if (digitalRead(BUTTON_PIN) == LOW && current.mode != MODE_ADDRESS && server.isConnected()) {
    current.mode = MODE_ADDRESS;
    newOrderReceived = true;
  }
}

/**
 * Initialization procedure
 */
void setup() {
  // Pins initialization
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // Pixels initialization
  pixels.begin();
  // Network initialization (skiped if the button is pushed)
  if (digitalRead(BUTTON_PIN) == HIGH) {
    server.registerServeMethod(listenToRequests);
    server.begin(mac);
  }
  // Watchdog initialization, only after long network intialization
  wdt_enable(WDTO_4S);
  // Random generator initialization
  randomSeed(analogRead(0));
  // Time (RTC clock) initialization
  setSyncProvider(RTC.get);
  // Initial mode selection
  if (!server.isConnected()) { 
    // No connection: demo mode
    current.mode = MODE_DEMO;
  } else {
    // Connection: back to last mode
    loadCurrent();
  }
}

/**
 * Main ever-running procedure
 */
void loop() {
  // Serve one HTTP request if any
  server.serve();
  // Check the button
  checkButton();
  // Display periodically
  static unsigned long lastDisplayEvent = 0;
  const unsigned long now = micros();
  const unsigned long next = lastDisplayEvent + DISPLAY_DELAY;
  if (now >= next) {
    display();
    lastDisplayEvent = now;
  }
  // Tell the watchdog the program is running
  wdt_reset();
}
