#include <Arduino.h>

#include "settings.h"
#include "defines.h"
#include "vars.h"

#include "debug.h"

Settings::Settings() {
  #ifdef ENABLE_EEPROM
    myEEPROM.start();      
  #endif
}

void Settings::load() {
  #ifdef ENABLE_EEPROM
    if (myEEPROM.hasSignature()) {
        nextView = myEEPROM.readByte(EEPROM_DISPLAY1_NEXT_VIEW);
        secondaryActiveView = myEEPROM.readByte(EEPROM_DISPLAY1_SECONDARY_VIEW);
        #ifdef ENABLE_SECOND_DISPLAY
            nextView2 = myEEPROM.readByte(EEPROM_DISPLAY2_NEXT_VIEW);
            secondaryActiveView2 = myEEPROM.readByte(EEPROM_DISPLAY2_SECONDARY_VIEW);
        #endif
    } else {
      myEEPROM.createSignature();
      setDefaults();
      save();
    }
  #else
    setDefaults();
  #endif
}

void Settings::save() {
    #ifdef ENABLE_EEPROM
    if (!myEEPROM.hasSignature()) {
      myEEPROM.createSignature();
    }
    myEEPROM.writeByte(EEPROM_DISPLAY1_NEXT_VIEW, myDisplays[1]->nextView);
    myEEPROM.writeByte(EEPROM_DISPLAY1_SECONDARY_VIEW, myDisplays[1]->secondaryActiveView);
    #ifdef ENABLE_SECOND_DISPLAY
      myEEPROM.writeByte(EEPROM_DISPLAY2_NEXT_VIEW, myDisplays[2]->nextView);
      myEEPROM.writeByte(EEPROM_DISPLAY2_SECONDARY_VIEW, myDisplays[2]->secondaryActiveView);
    #endif
  #endif
}

void Settings::setDefaults() {
    this->activeView = 1;
    this->secondaryActiveView = 0;
}

int Settings::getActiveView() {
    return this->activeView;
}
    
int Settings::getSecondaryActiveView() {
    return this->secondaryActiveView;
}
