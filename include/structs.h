#ifndef structs_h
#define structs_h

#include <Arduino.h>

typedef struct {
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t day;
  uint8_t mon;
  uint16_t year;
  uint8_t wday;
} S_DateTime;

typedef struct
{
  //int fuel;
  int kph;
  int rpm;
  int voltage;
  int intakeTemp;
  int ambientTemp;
  int coolantTemp;
  //int oilTemp;
  int timingAdvance;
} S_OBD_Data;

#endif