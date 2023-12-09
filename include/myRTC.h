#ifndef RTC_h
#define RTC_h

#include <Arduino.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

#include "defines.h"

class RTC_DS1302 {
  private:
    RtcDateTime dt;
    char dateString[DATE_LENGTH];
    char timeString[TIME_LENGTH];

  public:
    RTC_DS1302();
    void start();
    char* getFormattedDate();
    char* getFormattedTime();
    
};

#endif