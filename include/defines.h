#ifndef DEFINES_h
#define DEFINES_h

#define MOCK_OBD
//#define MOCK_OBD_LOW_VALUES
#define MOCK_OBD_NORMAL_VALUES
//#define MOCK_OBD_HIGH_VALUES

#ifdef MOCK_OBD_LOW_VALUES
    #define MOCK_OBD_batteryVoltage 118
    #define MOCK_OBD_kph 0
    #define MOCK_OBD_rpm 800
    #define MOCK_OBD_engineCoolantTemp 10
    #define MOCK_OBD_ambientAirTemp -5
    #define MOCK_OBD_intakeAirTemp 12
    #define MOCK_OBD_timingAdvance 5
#endif

#ifdef MOCK_OBD_NORMAL_VALUES
    #define MOCK_OBD_batteryVoltage 138
    #define MOCK_OBD_kph 50
    #define MOCK_OBD_rpm 3500
    #define MOCK_OBD_engineCoolantTemp 92
    #define MOCK_OBD_ambientAirTemp 25
    #define MOCK_OBD_intakeAirTemp 45
    #define MOCK_OBD_timingAdvance 12
#endif

#ifdef MOCK_OBD_HIGH_VALUES
    #define MOCK_OBD_batteryVoltage 148
    #define MOCK_OBD_kph 180
    #define MOCK_OBD_rpm 7500
    #define MOCK_OBD_engineCoolantTemp 120
    #define MOCK_OBD_ambientAirTemp 40
    #define MOCK_OBD_intakeAirTemp 95
    #define MOCK_OBD_timingAdvance 40
#endif

//#define ENABLE_EEPROM
#define ENABLE_OBD_BLUETOOTH
//#define ENABLE_STARTUP_LOGO
//#define ENABLE_RTC_CLOCK
//#define ENABLE_SECOND_DISPLAY

#define OBD_DEVICE_NAME (char*)"OBDII"
#define OBD_DEVICE_PIN (char*)"1234"

#define DELAY_KEYPAD 200
#define PIN_UP_KEY 27
#define PIN_DOWN_KEY 26
#define PIN_LEFT_KEY 25
#define PIN_RIGHT_KEY 33
#define PIN_ENTER_KEY 32

#define MAX_DISPLAYS 1

#define TFT1_DC 17
#define TFT1_CS 5

#define TFT2_DC 17
#define TFT2_CS 15

#define DELAY_MAIN_MENU_TASK 100
#define DELAY_MAIN_TASK 1000

#define DELAY_ODB 50
#define DELAY_READING 5

#define DATE_LENGTH 9
#define TIME_LENGTH 6

#define VIEW_MANAGER_DELAY 100
#define DELAY_VIEW_CHANGE 1000

#define DELAY_VIEW_RPM 100
#define DELAY_VIEW_KPH 500
#define DELAY_VIEW_BATTERY_VOLTAGE 500
#define DELAY_VIEW_COOLANT_TEMP 500
#define DELAY_VIEW_AMBIENT_AIR_TEMP 500
#define DELAY_VIEW_INTAKE_AIR_TEMP 500
#define DELAY_VIEW_DATE_TIME 1000
#define DELAY_VIEW_ADV 250

#define VIEW_NONE 0
#define VIEW_KPH 1
#define VIEW_RPM 2
#define VIEW_BATTERY_VOLTAGE 3
#define VIEW_COOLANT_TEMP 4
#define VIEW_AMBIENT_TEMP 5
#define VIEW_INTAKE_TEMP 6
#define VIEW_TIMING_ADV 7
#define VIEW_DATE_TIME 8
//#define VIEW_OIL_TEMP 9

#define TYPE_GAUGE_GRAPH 1
#define TYPE_DUAL_TEXT 2
#define TYPE_DATE 3

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
#define EEPROM_DISPLAY1_SECONDARY_VIEW 3
#define EEPROM_DISPLAY2_NEXT_VIEW 4
#define EEPROM_DISPLAY2_SECONDARY_VIEW 5

#define FRONT_COLOR WHITE
#define BACK_COLOR BLACK

#endif
