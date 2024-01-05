#include <Arduino.h>

#include "settings.h"
#include "defines.h"
#include "vars.h"

#include "debug.h"

#ifdef ENABLE_EEPROM
  #include "myEEPROM.h"    
#endif

Settings::Settings() {

  debug->println(DEBUG_LEVEL_DEBUG, "::Settings activated");

  #ifdef ENABLE_EEPROM
    myEEPROM = new MyEEPROM(512);
    myEEPROM->start();      
  #endif
}

void Settings::load() {
  debug->println(DEBUG_LEVEL_INFO, "Loading settings...");

  #ifdef ENABLE_EEPROM    
    if (myEEPROM->hasSignature()) {
        debug->println(DEBUG_LEVEL_INFO, "Signature OK");
        this->activeView = myEEPROM->readInt(EEPROM_DISPLAY1_NEXT_VIEW);
        this->secondaryActiveView = myEEPROM->readInt(EEPROM_DISPLAY1_SECONDARY_VIEW);
        #ifdef ENABLE_SECOND_DISPLAY
            nextView2 = myEEPROM->readInt(EEPROM_DISPLAY2_NEXT_VIEW);
            secondaryActiveView2 = myEEPROM->readInt(EEPROM_DISPLAY2_SECONDARY_VIEW);
        #endif
    } else {
      debug->println(DEBUG_LEVEL_INFO, "No signature");
      myEEPROM->createSignature();
      setDefaults();
      save();
    }
  #else
    setDefaults();
  #endif
  debug->println(DEBUG_LEVEL_DEBUG, "[OK] Load");
}

void Settings::save() {
  
  debug->println(DEBUG_LEVEL_DEBUG, "Saving settings...");
  #ifdef ENABLE_EEPROM
    if (!myEEPROM->hasSignature()) {
      debug->println(DEBUG_LEVEL_DEBUG, "No signature");
      myEEPROM->createSignature();
    }
    debug->println(DEBUG_LEVEL_DEBUG, "Writing data to EEPROM");
    myEEPROM->writeInt(EEPROM_DISPLAY1_NEXT_VIEW, this->activeView);
    myEEPROM->writeInt(EEPROM_DISPLAY1_SECONDARY_VIEW, this->secondaryActiveView);
    #ifdef ENABLE_SECOND_DISPLAY
      myEEPROM->writeInt(EEPROM_DISPLAY2_NEXT_VIEW, this->activeView2);
      myEEPROM->writeInt(EEPROM_DISPLAY2_SECONDARY_VIEW, this->secondaryActiveView2);
    #endif
  #endif
  debug->println(DEBUG_LEVEL_DEBUG, "[OK] Saving settings");
}

void Settings::setDefaults() {
    
    debug->println(DEBUG_LEVEL_INFO, "Setting default values");

    this->activeView = VIEW_COOLANT_TEMP;
    this->secondaryActiveView = VIEW_NONE;

    debug->println(DEBUG_LEVEL_DEBUG, "[OK] Setting default values");
}

int Settings::getActiveView(int displayID) {
    if (displayID == 1) {
      return this->activeView;
    } else if (displayID == 2) {
      return this->activeView2;
    } else {
      debug->println(DEBUG_LEVEL_ERROR, "Unknown display ID");
      return 0;
    }
}

void Settings::setActiveView(int displayID, int activeView) {
    if (displayID == 1) {
      this->activeView = activeView;
    } else if (displayID == 2) {
      this->activeView2 = activeView;
    } else {
      debug->println(DEBUG_LEVEL_ERROR, "Unknown display ID");
    }    
}
    
int Settings::getSecondaryActiveView(int displayID) {
    if (displayID == 1) {
      return this->secondaryActiveView;
    } else if (displayID == 2) {
      return this->secondaryActiveView2;
    } else {
      debug->println(DEBUG_LEVEL_ERROR, "Unknown display ID");
      return 0;
    }    
}

void Settings::setSecondaryActiveView(int displayID, int secondaryActiveView) {
    
    if (displayID == 1) {
      this->secondaryActiveView = secondaryActiveView;
    } else if (displayID == 2) {
      this->secondaryActiveView2 = secondaryActiveView;
    } else {
      debug->println(DEBUG_LEVEL_ERROR, "Unknown display ID");
    }
}
