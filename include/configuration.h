//////////////////
// OBD - ELM327 //
//////////////////
// Enable ELM327 (internal) debug logs
#define ENABLE_ELM327_DEBUG_LOGS

#define MODE_DEBUG_FULL
//#define MODE_DEBUG
//#define MODE_RELEASE

// Mock OBD for testing without real OBD device 
//#define MOCK_OBD

// Mock values for testing without real OBD device //
// Enable only one of the follow
// Values are in defines.h

#define MOCK_OBD_LOW_VALUES
//#define MOCK_OBD_NORMAL_VALUES
//#define MOCK_OBD_HIGH_VALUES

////////////
// Gauges //
////////////
#define GAUGE_BATTERY_VOLTAGE
#define GAUGE_KMH
#define GAUGE_RPM
#define GAUGE_COOLANT_TEMP
#define GAUGE_INTAKE_TEMP
#define GAUGE_TIMING_ADV
#define GAUGE_ENGINE_LOAD
#define GAUGE_SHORT_FUEL_TRIM
#define GAUGE_LONG_FUEL_TRIM
#define GAUGE_THROTTLE
//#define GAUGE_MAF_RATE
//#define GAUGE_FUEL_LEVEL
//#define GAUGE_AMBIENT_TEMP
//#define GAUGE_OIL_TEMP
//#define GAUGE_ABS_LOAD
#define SECONDARY_GAUGE_KMH
#define SECONDARY_GAUGE_RPM
#define SECONDARY_GAUGE_COOLANT_TEMP
#define SECONDARY_GAUGE_INTAKE_TEMP
#define SECONDARY_GAUGE_SHORT_FUEL_TRIM
#define SECONDARY_GAUGE_LONG_FUEL_TRIM
//#define SECONDARY_GAUGE_ENGINE_LOAD
//#define SECONDARY_GAUGE_THROTTLE

//////////////
// Settings //
//////////////

#define DEBUG_MODE
#define ENABLE_SECONDARY_VIEWS

// Enable EEPROM to save settings in EEPROM
#define ENABLE_EEPROM
//#define CLEAR_SETTINGS

#ifdef ESP32
    // Dual Core
    #define USE_MULTI_THREAD
#endif

#ifdef ESP8266
    //
    #define DRAW_FAST
#endif


/////////////////
// Date - Time //
/////////////////
// Enable RTC clock module 
//#define ENABLE_RTC_CLOCK


/////////////
// Display //
/////////////
// Show start up logo until connect to OBD
//#define ENABLE_STARTUP_LOGO

// Enable the 2nd TFT screen
#define ENABLE_SECOND_DISPLAY


////////////////
// Connection //
////////////////
// Select connection method (only one of the follow) 

#ifdef ESP32
    #define USE_OBD_BLUETOOTH
    //#define USE_OBD_WIFI
#endif

#ifdef ESP8266
    #define USE_OBD_WIFI
#endif


////////////
// Keypad //
////////////
//#define USE_MOCK_KEYPAD
#define BUTTON_PRESSED HIGH



