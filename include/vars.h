#ifndef VARS_h
#define VARS_h

#include <Arduino.h>

#ifdef ENABLE_OBD_BLUETOOTH
  #include <BluetoothSerial.h>
  #include "bluetoothOBD.h"
#endif

#include <ELMduino.h>

#include "debug.h"
#include "displays.h"
#include "gauge.h"
#include "structs.h"
#include "settings.h"

#ifdef ENABLE_EEPROM  
  #include "myEEPROM.h"
#endif

#ifdef ENABLE_RTC_CLOCK
  #include "myRTC.h"
#endif

extern int activeDisplay;
extern ELM327 *obd;

#ifdef USE_MULTI_THREAD
extern SemaphoreHandle_t keyPadSemaphore;
extern SemaphoreHandle_t btConnectedSemaphore;
extern SemaphoreHandle_t obdConnectedSemaphore;
extern SemaphoreHandle_t obdValueSemaphore;

// Tasks
extern TaskHandle_t t_core0_tft1;
extern TaskHandle_t t_core0_tft2;
extern TaskHandle_t t_core0_keypad;
extern TaskHandle_t t_core1_obd;
#endif

extern Debug *debug;
extern Displays *myDisplays[MAX_DISPLAYS];
extern Gauge *myGauges[MAX_VIEWS + 1];

extern char dateString[DATE_LENGTH];
extern char timeString[TIME_LENGTH];
extern char oldDateString[DATE_LENGTH];
extern char oldTimeString[TIME_LENGTH];

extern Settings *mySettings;

#ifdef ENABLE_OBD_BLUETOOTH
    extern BluetoothSerial SerialBT;
    extern BluetoothOBD *bluetoothOBD;
    extern String deviceName[8];
    extern String deviceAddr[8];
    extern int btDeviceCount;
    extern String obdDeviceName;
    extern esp_bd_addr_t client_addr;
    extern esp_spp_sec_t sec_mask;
    extern esp_spp_role_t role;

#endif

#ifdef ENABLE_EEPROM
    extern MyEEPROM myEEPROM;
#endif

#ifdef ENABLE_RTC_CLOCK
    extern RTC_DS1302 myRTC;
#endif

#endif