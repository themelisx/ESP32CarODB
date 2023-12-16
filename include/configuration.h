//////////////////
// OBD - ELM327 //
//////////////////
// Enable ELM327 (internal) debug logs
//#define ENABLE_ELM327_DEBUG_LOGS

// Mock OBD for testing without real OBD device 
//#define MOCK_OBD

// Mock values for testing without real OBD device //
// Enable only one of the follow
// Values are in defines.h

//#define MOCK_OBD_LOW_VALUES
#define MOCK_OBD_NORMAL_VALUES
//#define MOCK_OBD_HIGH_VALUES


//////////////
// Settings //
//////////////
// Enable EEPROM to save settings in EEPROM
//#define ENABLE_EEPROM


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
//#define ENABLE_SECOND_DISPLAY


////////////////
// Connection //
////////////////
// Select connection method (only one of the follow) 
#define USE_OBD_BLUETOOTH
//#define USE_OBD_WIFI


////////////
// Keypad //
////////////
#define USE_MOCK_KEYPAD



