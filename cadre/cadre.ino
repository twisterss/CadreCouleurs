// This is the main sketch of the color frame.
// It contains a control web server and the display control.
// Think of loading colors in the EEPROM before using this sketch.

#include <avr/wdt.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>  
#include "RTCControl.h"
#include <Time.h> 
#include <Adafruit_NeoPixel.h>
#include "Pixels.h"
#include "WebServer.h"
#include <EEPROM.h>

// Hardware configuration
#define STRIP_PIN_DATA 6
#define STRIP_PIN_POWER A3
#define BUTTON_PIN 7

// Minimum delay between two display states (microseconds)
#define DISPLAY_DELAY 1000
// Maximum number of characters for a displayed message
#define MAX_TEXT_SIZE 30

// Number of colors stored in EEPROM
#define COLOR_HUES 85
#define COLOR_SATS 4

// Working modes
#define MODE_OFF 0
#define MODE_CLOCK 1
#define MODE_COLOR 2
#define MODE_GRADIENT 3
#define MODE_RANDOM 4
#define MODE_TEXT 5
#define MODE_DEMO 6
#define MODE_ALARM 7
#define MODE_RAINBOW 8
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
  uint8_t hours;
  uint8_t minutes;
  char text[MAX_TEXT_SIZE];
  uint8_t speed;
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
 * Mix 2 colors together.
 * If step = 0, this is color1, if step = maxStep, this is color2, between, this is a mix
 */
uint32_t transitionColor(uint8_t step, uint8_t maxStep, uint32_t color1, uint32_t color2 = 0) {
  uint16_t r1, g1, b1, r2, g2, b2;
  r1 = (uint8_t) (color1 >> 16); g1 = (uint8_t) (color1 >> 8); b1 = (uint8_t) color1; 
  r2 = (uint8_t) (color2 >> 16); g2 = (uint8_t) (color2 >> 8); b2 = (uint8_t) color2; 
  r1 *= maxStep - step; g1*= maxStep - step; b1 *= maxStep - step;
  r2 *= step; g2*= step; b2 *= step;
  r1+= r2; g1+= g2; b1+= b2;
  r1/= maxStep; g1/= maxStep; b1/= maxStep; 
  return pixels.color(r1, g1, b1);
}

/**
 * Load a color value from EEPROM
 * depending on discrete hue and saturation values.
 * Brightness is computed and can be any value from 0 to 255.
 */
uint32_t loadColor(uint8_t hue, uint8_t sat = COLOR_SATS-1, uint8_t bright = 255) {
  uint16_t colorInd = (hue * COLOR_SATS + sat) * 3;
  return transitionColor(255-bright, 255, pixels.color(EEPROM.read(colorInd), EEPROM.read(colorInd + 1), EEPROM.read(colorInd + 2)));
}

/**
 * Return a random color with a minimum hue
 * distance from the last random color.
 * The brightness range can be changed.
 */
uint32_t randomColor(uint8_t minDist = 0, uint8_t minBright = 0, uint8_t maxBright = 255) {
  static uint8_t oldHue = 0;
  uint8_t hue, sat, bright;
  uint16_t topBright;
  do {
    // Choose a color
    hue = random(COLOR_HUES);
  } while (!((hue <= oldHue && (oldHue - hue) >= minDist && (hue + COLOR_HUES - oldHue) >= minDist) ||
    (hue > oldHue && (hue - oldHue >= minDist) && (oldHue + COLOR_HUES - hue) >= minDist)));
  oldHue = hue;
  sat = random(COLOR_SATS);
  topBright = maxBright;
  topBright+=1;
  bright = random(minBright, topBright);
  return loadColor(hue, sat, bright);
}

/**
 * Print a fixed text on the display
 */
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
 * Print the current time on the display
 */
void fixedTime(uint32_t hoursColor, uint32_t minutesColor, uint32_t backgroundColor = 0) {
    pixels.clear(backgroundColor);
    char tmpChars[3];
    String tmpString;
    tmpString = String(hour());
    if (tmpString.length() < 2)
      tmpString = "0" + tmpString;
    tmpString.toCharArray(tmpChars, 3);
    fixedText(tmpChars, hoursColor, 0, 0);
    tmpString = String(minute());
    if (tmpString.length() < 2)
      tmpString = "0" + tmpString;
    tmpString.toCharArray(tmpChars, 3);
    fixedText(tmpChars, minutesColor, 1, 5);
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
    fixedTime(current.color1, current.color2);
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
  if (newDisplay || remaining == 0) {
    // Select a color
    color = randomColor(10, 255, 255);
    // Count the number of pixels to change
    remaining = 0;
    for (uint8_t x = 0; x < pixels.width(); x++)
      for (uint8_t y = 0; y < pixels.height(); y++)
        if (pixels.get(x, y) != color)
          remaining++;
  }
  if (skipSteps(300)) {
    // Find a pixel to change
    uint8_t toChange = random(remaining);
    bool done  = false;
    for (uint8_t x = 0; x < pixels.width() && !done; x++) {
      for (uint8_t y = 0; y < pixels.height() && !done; y++) {
        if (pixels.get(x, y) != color) {
          if (toChange == 0) {
            pixels.set(x, y, color);
            done = true;
          }
          toChange--;
        }
      }
    }
    pixels.commit();
    remaining--;
  }
}

/**
 * Display some geometric shapes
 */
void displayRandom(bool newDisplay) {
  if (skipSteps(5)) {
    pixels.set(random(pixels.width()), random(pixels.height()), randomColor(1, 50));
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
  if (duration >= 10000) {
    // Change the mode
    if (mode == MODE_RANDOM) {
      mode = MODE_TEXT;
      strcpy(current.text, "Cadre couleurs");
      current.color1 = pixels.color(255, 255, 255);
      current.color2 = pixels.color(0, 0, 0);
    } else if (mode == MODE_TEXT) {
      current.speed = 255;
      mode = MODE_RAINBOW;
    } else if (mode == MODE_RAINBOW) {
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
    case MODE_RAINBOW:
      displayRainbow(newMode);
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
 * Start displaying the time at a certain hour
 */
void displayAlarm(bool newDisplay) {
  static uint8_t darkness = 0;
  static uint16_t onDelay = 0;
  static bool inAlarm = false;
  if (newDisplay) {
   darkness = 0;
   onDelay = 6;
  }
  if (newDisplay || skipSteps(1000)) {
    if (onDelay > 0) {
      onDelay--;
      if (darkness > 0 && (onDelay & 1))
        darkness--;
      if (onDelay == 0) {
        pixels.switchOff();
      } else {
        fixedTime(transitionColor(darkness, 150, current.color1), transitionColor(darkness, 150, current.color1), transitionColor(darkness, 150, current.color2));
        pixels.commit();
      }
    }
  }
  if (current.hours == hour() && current.minutes == minute()) {
    if (!inAlarm) {
      // New alarm detected
      darkness = 150;
      onDelay = (60 + 5) * 60 + 1;
      inAlarm = true;
    }
  } else {
    inAlarm = false;
  }
}

/**
 * Display a rainbow
 */
void displayRainbow(bool newDisplay) {
  static uint8_t colorInd = 0, trans = 0, transMax;
  uint32_t color;
  int8_t diag, x, y;
  if (newDisplay) {
    // Set the speed by changing the number of transitions
    uint8_t newTransMax = 90 - (current.speed / 3);
    trans = (trans * newTransMax) / transMax;
    transMax = newTransMax;
  }
  if (skipSteps(20)) {
    // Update pixels by the diagonal
    for (diag = 0; diag <= pixels.width() + pixels.height() - 2; diag++) {
      color = transitionColor(trans, transMax, loadColor((colorInd + (diag)*2) % COLOR_HUES), loadColor((colorInd + (diag+1)*2) % COLOR_HUES));
      if (diag >= pixels.height()) {
        y = pixels.height() - 1;
        x = diag - y;
      } else {
        x = 0;
        y = diag;
      }
      while(x < pixels.width() && y >= 0) {
        pixels.set(x, y, color);
        x+= 1;
        y-= 1;
      }
    }
    // Get ready for next step
    trans++;
    if (trans >= transMax) {
      trans = 0;
      colorInd+=2;
      if (colorInd >= COLOR_HUES)
        colorInd = 0;
    }
    pixels.commit();
  }
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
    case MODE_ALARM:
      displayAlarm(newOrderReceived);
      break;
    case MODE_RAINBOW:
      displayRainbow(newOrderReceived);
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
    if (current.mode == MODE_CLOCK || current.mode == MODE_COLOR || current.mode == MODE_TEXT || current.mode == MODE_ALARM)
      current.color1 = pixels.color(request.params.substring(2, 5).toInt(), request.params.substring(5, 8).toInt(), request.params.substring(8, 11).toInt());
    if (current.mode == MODE_CLOCK || current.mode == MODE_TEXT || current.mode == MODE_ALARM)
      current.color2 = pixels.color(request.params.substring(11, 14).toInt(), request.params.substring(14, 17).toInt(), request.params.substring(17, 20).toInt());
    if (current.mode == MODE_TEXT)
      request.params.substring(20).toCharArray(current.text, MAX_TEXT_SIZE);
    if (current.mode == MODE_ALARM) {
      current.hours = request.params.substring(20, 22).toInt();
      current.minutes = request.params.substring(22, 24).toInt();
    }
    if (current.mode == MODE_RAINBOW)
      current.speed = request.params.substring(2, 5).toInt();
    if (current.mode == MODE_CLOCK || current.mode == MODE_ALARM) {
      // Set the current time
      time_t time = request.params.substring(current.mode == MODE_CLOCK ? 20 : 24).toInt();
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
  const unsigned long timeNow = micros();
  const unsigned long duration = timeNow - lastDisplayEvent;
  if (duration >= DISPLAY_DELAY) {
    display();
    lastDisplayEvent = timeNow;
    // Tell the watchdog the program is running properly
    wdt_reset();
  }
}
