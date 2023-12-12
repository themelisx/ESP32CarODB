#include <Arduino.h>

#include "settings.h"
#include "defines.h"
#include "vars.h"

#include "debug.h"

Settings::Settings() {

  debug->println(DEBUG_LEVEL_DEBUG, "::Settings activated");

  #ifdef ENABLE_EEPROM
    myEEPROM.start();      
  #endif
}

void Settings::load() {
  debug->println(DEBUG_LEVEL_INFO, "Loading settings...");

  #ifdef ENABLE_EEPROM    
    if (myEEPROM.hasSignature()) {
        debug->println(DEBUG_LEVEL_INFO, "Signature OK");
        nextView = myEEPROM.readByte(EEPROM_DISPLAY1_NEXT_VIEW);
        secondaryActiveView = myEEPROM.readByte(EEPROM_DISPLAY1_SECONDARY_VIEW);
        #ifdef ENABLE_SECOND_DISPLAY
            nextView2 = myEEPROM.readByte(EEPROM_DISPLAY2_NEXT_VIEW);
            secondaryActiveView2 = myEEPROM.readByte(EEPROM_DISPLAY2_SECONDARY_VIEW);
        #endif
    } else {
      debug->println(DEBUG_LEVEL_INFO, "No signature");
      myEEPROM.createSignature();
      setDefaults();
      save();
    }
  #else
    setDefaults();
  #endif
  debug->println(DEBUG_LEVEL_INFO, "[OK] Load");
}

void Settings::save() {
  
  debug->println(DEBUG_LEVEL_INFO, "Saving settings...");
  #ifdef ENABLE_EEPROM
    if (!myEEPROM.hasSignature()) {
      debug->println(DEBUG_LEVEL_DEBUG, "No signature");
      myEEPROM.createSignature();
    }
    debug->println(DEBUG_LEVEL_DEBUG, "Writing data to EEPROM");
    myEEPROM.writeByte(EEPROM_DISPLAY1_NEXT_VIEW, myDisplays[1]->nextView);
    myEEPROM.writeByte(EEPROM_DISPLAY1_SECONDARY_VIEW, myDisplays[1]->secondaryActiveView);
    #ifdef ENABLE_SECOND_DISPLAY
      myEEPROM.writeByte(EEPROM_DISPLAY2_NEXT_VIEW, myDisplays[2]->nextView);
      myEEPROM.writeByte(EEPROM_DISPLAY2_SECONDARY_VIEW, myDisplays[2]->secondaryActiveView);
    #endif
  #endif
  debug->println(DEBUG_LEVEL_INFO, "[OK] Saving settings");
}

void Settings::setDefaults() {
    
    debug->println(DEBUG_LEVEL_INFO, "Setting default values");

    this->activeView = VIEW_KPH;
    this->secondaryActiveView = VIEW_NONE;

    debug->println(DEBUG_LEVEL_DEBUG, "[OK] Setting default values");
}

int Settings::getActiveView() {
    return this->activeView;
}
    
int Settings::getSecondaryActiveView() {
    return this->secondaryActiveView;
}
