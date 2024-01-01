#ifndef DEFINES_h
#define DEFINES_h

#include "configuration.h"

#ifdef MOCK_OBD
    #ifdef MOCK_OBD_LOW_VALUES
        #define MOCK_OBD_batteryVoltage 118
        #define MOCK_OBD_kph 0
        #define MOCK_OBD_rpm 800
        #define MOCK_OBD_engineCoolantTemp 10    
        #define MOCK_OBD_intakeAirTemp 12
        #define MOCK_OBD_timingAdvance 5
        #define MOCK_OBD_engineLoad 15
        #define MOCK_OBD_mafRate 2
        #define MOCK_OBD_shortFuelTrim -15
        #define MOCK_OBD_longFuelTrim -20
        #define MOCK_OBD_throttle 10
        #define MOCK_OBD_fuelLevel 15
        #define MOCK_OBD_ambientAirTemp -5
        #define MOCK_OBD_oilTemp -10
        #define MOCK_OBD_absLoad 12
    #endif

    #ifdef MOCK_OBD_NORMAL_VALUES
        #define MOCK_OBD_batteryVoltage 138
        #define MOCK_OBD_kph 50
        #define MOCK_OBD_rpm 3500
        #define MOCK_OBD_engineCoolantTemp 92
        #define MOCK_OBD_intakeAirTemp 35
        #define MOCK_OBD_timingAdvance 12
        #define MOCK_OBD_engineLoad 15
        #define MOCK_OBD_mafRate 5
        #define MOCK_OBD_shortFuelTrim 2
        #define MOCK_OBD_longFuelTrim 5
        #define MOCK_OBD_throttle 35
        #define MOCK_OBD_fuelLevel 50
        #define MOCK_OBD_ambientAirTemp 25
        #define MOCK_OBD_oilTemp 45
        #define MOCK_OBD_absLoad 25
    #endif

    #ifdef MOCK_OBD_HIGH_VALUES
        #define MOCK_OBD_batteryVoltage 148
        #define MOCK_OBD_kph 180
        #define MOCK_OBD_rpm 7500
        #define MOCK_OBD_engineCoolantTemp 120
        #define MOCK_OBD_intakeAirTemp 95
        #define MOCK_OBD_timingAdvance 40
        #define MOCK_OBD_engineLoad 85
        #define MOCK_OBD_mafRate 7
        #define MOCK_OBD_shortFuelTrim 15
        #define MOCK_OBD_longFuelTrim 20
        #define MOCK_OBD_throttle 81
        #define MOCK_OBD_fuelLevel 100
        #define MOCK_OBD_ambientAirTemp 45
        #define MOCK_OBD_oilTemp 125
        #define MOCK_OBD_absLoad 85
    #endif
#endif

#ifdef ENABLE_ELM327_DEBUG_LOGS
    #define OBD_DEBUG_LOGS true
#else
    #define OBD_DEBUG_LOGS false
#endif

#define ODB_TIMEOUT_MS 100
#define OBD_DEVICE_NAME (char*)"OBDII"
#define OBD_DEVICE_PIN (char*)"1234"

#define DELAY_KEYPAD 200

#ifdef ESP32
    #define PIN_UP_KEY 27
    #define PIN_DOWN_KEY 26
    #ifdef ENABLE_SECOND_DISPLAY
        #define PIN_LEFT_KEY 25
        #define PIN_RIGHT_KEY 33
        #define PIN_ENTER_KEY 32
    #endif
#endif
#ifdef ESP8266
    #define PIN_UP_KEY D1
    #define PIN_DOWN_KEY D2
#endif

#define MAX_DISPLAYS 1

#define DELAY_BT_DISCOVER_TIME 5000


#ifdef ESP32
    #define TFT1_CS 5
    #define TFT1_DC 17
    #define TFT2_CS 15
    #define TFT2_DC 17    
#endif

#ifdef ESP8266
    #define TFT1_CS D0
    #define TFT1_DC D3
    #define TFT2_CS D4
    #define TFT2_DC D1    
#endif

#define TFT1_HEIGHT 240
#define TFT1_WIDTH 240
#define TFT2_HEIGHT 240
#define TFT2_WIDTH 240

#define TEST_KEY_DELAY 10000
#define DELAY_MAIN_MENU_TASK 100
#define DELAY_MAIN_TASK 1000
#define DELAY_REFRESH_VIEW 100

#define DELAY_ODB 50
#define DELAY_READING 10

#define DATE_LENGTH 9
#define TIME_LENGTH 6

#define VIEW_MANAGER_DELAY 100
#define DELAY_VIEW_CHANGE 1000

#define DELAY_VIEW_BATTERY_VOLTAGE 500
#define DELAY_VIEW_KPH 500
#define DELAY_VIEW_RPM 50
#define DELAY_VIEW_COOLANT_TEMP 500
#define DELAY_VIEW_INTAKE_AIR_TEMP 500
#define DELAY_VIEW_TIMING_ADV 100
#define DELAY_VIEW_ENGINE_LOAD 300
#define DELAY_VIEW_MAF_RATE 100
#define DELAY_VIEW_SHORT_FUEL_TRIM 100
#define DELAY_VIEW_LONG_FUEL_TRIM 100
#define DELAY_VIEW_THROTTLE 100
#define DELAY_VIEW_FUEL_LEVEL 1000
#define DELAY_VIEW_AMBIENT_AIR_TEMP 1000
#define DELAY_VIEW_OIL_TEMP 1000
#define DELAY_VIEW_ABS_LOAD 100

//#define DELAY_VIEW_DATE_TIME 1000

#define MAX_VIEWS 10
#define MAX_SECONDARY_VIEWS 4

#define VIEW_NONE 0
#define VIEW_BATTERY_VOLTAGE 1
#define VIEW_KPH 2
#define VIEW_RPM 3
#define VIEW_COOLANT_TEMP 4
#define VIEW_INTAKE_TEMP 5
#define VIEW_TIMING_ADV 6
#define VIEW_ENGINE_LOAD 7
#define VIEW_SHORT_FUEL_TRIM 8
#define VIEW_LONG_FUEL_TRIM 9
#define VIEW_THROTTLE 10
#define VIEW_MAF_RATE 11
//supportedPIDs_21_40
#define VIEW_FUEL_LEVEL 12
//supportedPIDs_41_60
#define VIEW_AMBIENT_TEMP 13
#define VIEW_OIL_TEMP 14
#define VIEW_ABS_LOAD 15

//#define VIEW_DATE_TIME 99

#define TYPE_GAUGE_GRAPH 1
#define TYPE_SIMPLE_TEXT 2
#define TYPE_DUAL_TEXT 3
#define TYPE_DATE 4

#define NO_IMAGE -1

#define STATE_UNKNOWN -1
#define STATE_LOW 0
#define STATE_NORMAL 1
#define STATE_HIGH 2
#define STATE_OUT_OF_RANGE 3

#define DATE_LENGTH 9
#define TIME_LENGTH 6

#define WHITE GC9A01A_WHITE
#define BLACK GC9A01A_BLACK
#define RED GC9A01A_RED
#define GREEN GC9A01A_GREEN
#define BLUE GC9A01A_BLUE
#define ORANGE GC9A01A_ORANGE

#define EEPROM_DISPLAY1_NEXT_VIEW 2
#define EEPROM_DISPLAY1_SECONDARY_VIEW 4
#define EEPROM_DISPLAY2_NEXT_VIEW 6
#define EEPROM_DISPLAY2_SECONDARY_VIEW 8

#define FRONT_COLOR WHITE
#define BACK_COLOR BLACK

#endif
