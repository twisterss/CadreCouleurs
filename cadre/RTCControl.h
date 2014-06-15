/*
 * RTC library copied and merged from different sources.
 * Able to read/write time or bytes in memory
 * 
 * Original by Michael Margolis (2009)
 */

#ifndef DS1307RTC_h
#define DS1307RTC_h

#include <Time.h>

// library interface description
class RTCControl
{
  // user-accessible "public" interface
  public:
    RTCControl();
    // Get current time
    static time_t get();
    // Set current time
    static bool set(time_t t);
    // Get current time
    static bool read(tmElements_t &tm);
    // Set current time
    static bool write(tmElements_t &tm);
    // Read bytes in RAM (address from 0 to 55)
    static void readBytesInRam(uint8_t address, uint8_t length, uint8_t* p_data);
    // Write bytes in RAM (address from 0 to 55)
    static void writeBytesInRam(uint8_t address, uint8_t length, uint8_t* p_data);
    // Presence of the RTC chip
    static bool chipPresent() { return exists; }

  private:
    static bool exists;
    static uint8_t dec2bcd(uint8_t num);
    static uint8_t bcd2dec(uint8_t num);
};

#ifdef RTC
#undef RTC // workaround for Arduino Due, which defines "RTC"...
#endif

extern RTCControl RTC;

#endif
 

