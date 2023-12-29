#ifndef VARS_h
#define VARS_h

#include <Arduino.h>

#ifdef USE_OBD_BLUETOOTH
  #include <BluetoothSerial.h>  
#endif

#include "odbAdapter.h"

#include <ELMduino.h>

#include "debug.h"
#include "displays.h"
#include "gauge.h"
#include "structs.h"
#include "settings.h"

#ifdef ENABLE_RTC_CLOCK
  #include "myRTC.h"
#endif

extern ELM327 *obd;

#ifdef USE_MULTI_THREAD
extern SemaphoreHandle_t semaphoreActiveDisplay;
extern SemaphoreHandle_t semaphoreActiveView;
extern SemaphoreHandle_t semaphoreData;
extern SemaphoreHandle_t btConnectedSemaphore;
extern SemaphoreHandle_t obdConnectedSemaphore;

// Tasks
extern TaskHandle_t t_core0_tft1;
extern TaskHandle_t t_core0_tft2;
extern TaskHandle_t t_core0_keypad;
extern TaskHandle_t t_core1_obd;

extern int realActiveDisplay;
extern int getActiveDisplay();
#else
extern int activeDisplay;
#endif

extern Debug *debug;
extern Displays *myDisplays[MAX_DISPLAYS];
extern Gauge *myGauges[MAX_VIEWS + 1];

extern Settings *mySettings;
extern OdbAdapter *odbAdapter;

#ifdef ENABLE_RTC_CLOCK
    extern RTC_DS1302 myRTC;

    extern char dateString[DATE_LENGTH];
    extern char timeString[TIME_LENGTH];
    extern char oldDateString[DATE_LENGTH];
    extern char oldTimeString[TIME_LENGTH];
#endif

#endif