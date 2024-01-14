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

    int activeView2;
    int secondaryActiveView2;

  public:
    Settings();

    void load();
    void save();
    void setDefaults();

    int getActiveView(int displayID);
    int getSecondaryActiveView(int displayID);

    void setActiveView(int displayID, int activeView);
    void setSecondaryActiveView(int displayID, int secondaryActiveView);
};

#endif