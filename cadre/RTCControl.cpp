#include <Wire.h>
#include "RTCControl.h"

#define DS1307_CTRL_ID 0x68 

RTCControl::RTCControl()
{
  Wire.begin();
}
  
// PUBLIC FUNCTIONS
time_t RTCControl::get()   // Aquire data from buffer and convert to time_t
{
  tmElements_t tm;
  if (read(tm) == false) return 0;
  return(makeTime(tm));
}

bool RTCControl::set(time_t t)
{
  tmElements_t tm;
  breakTime(t, tm);
  tm.Second |= 0x80;  // stop the clock 
  write(tm); 
  tm.Second &= 0x7f;  // start the clock
  write(tm); 
}

// Aquire data from the RTC chip in BCD format
bool RTCControl::read(tmElements_t &tm)
{
  uint8_t sec;
  Wire.beginTransmission(DS1307_CTRL_ID);
  Wire.write((uint8_t)0x00); 
  if (Wire.endTransmission() != 0) {
    exists = false;
    return false;
  }
  exists = true;

  // request the 7 data fields   (secs, min, hr, dow, date, mth, yr)
  Wire.requestFrom(DS1307_CTRL_ID, tmNbrFields);
  if (Wire.available() < tmNbrFields) return false;
  sec = Wire.read();
  tm.Second = bcd2dec(sec & 0x7f);   
  tm.Minute = bcd2dec(Wire.read() );
  tm.Hour =   bcd2dec(Wire.read() & 0x3f);  // mask assumes 24hr clock
  tm.Wday = bcd2dec(Wire.read() );
  tm.Day = bcd2dec(Wire.read() );
  tm.Month = bcd2dec(Wire.read() );
  tm.Year = y2kYearToTm((bcd2dec(Wire.read())));
  if (sec & 0x80) return false; // clock is halted
  return true;
}

bool RTCControl::write(tmElements_t &tm)
{
  Wire.beginTransmission(DS1307_CTRL_ID);
  Wire.write((uint8_t)0x00); // reset register pointer  
  Wire.write(dec2bcd(tm.Second)) ;   
  Wire.write(dec2bcd(tm.Minute));
  Wire.write(dec2bcd(tm.Hour));      // sets 24 hour format
  Wire.write(dec2bcd(tm.Wday));   
  Wire.write(dec2bcd(tm.Day));
  Wire.write(dec2bcd(tm.Month));
  Wire.write(dec2bcd(tmYearToY2k(tm.Year))); 
  if (Wire.endTransmission() != 0) {
    exists = false;
    return false;
  }
  exists = true;
  return true;
}

void RTCControl::readBytesInRam(uint8_t address, uint8_t length, uint8_t* p_data) {
    Wire.beginTransmission(DS1307_CTRL_ID);
    Wire.write(address+8);    
    Wire.endTransmission();
 
    Wire.requestFrom(DS1307_CTRL_ID, (int)length);
    for (uint8_t i = 0; i < length; i++) {
        p_data[i] = Wire.read();
    }
    Wire.endTransmission();
}

void RTCControl::writeBytesInRam(uint8_t address, uint8_t length, uint8_t* p_data) {
    Wire.beginTransmission(DS1307_CTRL_ID);
    Wire.write(address+8);    
    for (uint8_t i = 0; i < length; i++) {
        Wire.write(p_data[i]);
    }
    Wire.endTransmission();
}

// PRIVATE FUNCTIONS

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t RTCControl::dec2bcd(uint8_t num)
{
  return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t RTCControl::bcd2dec(uint8_t num)
{
  return ((num/16 * 10) + (num % 16));
}

bool RTCControl::exists = false;

RTCControl RTC = RTCControl(); // create an instance for the user

