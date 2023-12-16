#ifndef SETTINGS_h
#define SETTINGS_h

#include <Arduino.h>

#include "defines.h"

#ifdef ENABLE_EEPROM
  #include "myEEPROM.h"    
#endif

class Settings {
  private:

    #ifdef ENABLE_EEPROM
      MyEEPROM *myEEPROM;
    #endif

    bool status;

    int activeView;
    int secondaryActiveView;

  public:
    Settings();

    void load();
    void save();
    void setDefaults();

    int getActiveView();
    int getSecondaryActiveView();
};

#endif