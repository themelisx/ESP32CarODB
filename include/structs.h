#ifndef structs_h
#define structs_h

#include <Arduino.h>

  #ifdef ENABLE_RTC_CLOCK

    typedef struct {
      uint8_t hour;
      uint8_t min;
      uint8_t sec;
      uint8_t day;
      uint8_t mon;
      uint16_t year;
      uint8_t wday;
    } S_DateTime;
    
  #endif


#endif