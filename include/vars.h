#ifndef VARS_h
#define VARS_h

#include <Arduino.h>

#ifdef USE_OBD_BLUETOOTH
  #include <BluetoothSerial.h>  
#endif

#include "odbAdapter.h"

#include <ELMduino.h>

#include "debug.h"
#include "structs.h"
#include "settings.h"
#include "displayManager.h"

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
#endif

extern Debug *debug;
extern DisplayManager *displayManager;

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