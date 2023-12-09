#include <Arduino.h>
#include "defines.h"

#ifdef ENABLE_EEPROM  

#include "myeeprom.h"
#include <EEPROM.h>


#include "vars.h"

#include "debug.h"

MyEEPROM::MyEEPROM(int eepromSize) {
      this->size = eepromSize;
      active = false;
};

bool MyEEPROM::start() {
  debug.print(DEBUG_LEVEL_INFO, F("Starting EEPROM with "));
  debug.print(DEBUG_LEVEL_INFO, this->size);
  debug.println(DEBUG_LEVEL_INFO, F(" bytes... "));
  
  if (EEPROM.begin(this->size)) {
    debug.println(DEBUG_LEVEL_INFO, F("Done"));
    active = true;
  } else {
    debug.println(DEBUG_LEVEL_ERROR, F("Failed"));
    active = false;
  }
  return active;
}

void MyEEPROM::showOutOfBoundsError(int address, int size) {

  showOutOfBoundsError(address);
  debug.print(DEBUG_LEVEL_ERROR, F("Out of bounds (size): "));
  debug.println(DEBUG_LEVEL_ERROR, size);
}

void MyEEPROM::showOutOfBoundsError(int address) {

  debug.print(DEBUG_LEVEL_ERROR, F("Cannot access EEPROM at address "));
  debug.println(DEBUG_LEVEL_ERROR, address);  
}

void MyEEPROM::showNotActive() {
  debug.println(DEBUG_LEVEL_ERROR, F("EEPROM module is not active. Aborting"));
}

void MyEEPROM::createSignature() {
  debug.println(DEBUG_LEVEL_DEBUG, F("Creating signature to EEPROM..."));

  if (!active) {
    showNotActive();
    return;
  }

  EEPROM.write(0, 'G');
  EEPROM.write(1, 'R');
  if (EEPROM.commit()) {
    debug.println(DEBUG_LEVEL_DEBUG, F("Done"));
  } else {
    debug.println(DEBUG_LEVEL_ERROR, F("Cannot write to EEPROM"));
  }
}

bool MyEEPROM::hasSignature() {

  if (!active) {
    showNotActive();
    return false;
  }

  return ((EEPROM.read(0) == 'G') && (EEPROM.read(1) == 'R'));
}

byte MyEEPROM::readByte(int address) {  
  if (!active) {
    showNotActive();
    return 0;
  }

  if (address < this->size) {
    return EEPROM.read(address);
  } else {
    showOutOfBoundsError(address);
    return 0;
  }
}

void MyEEPROM::writeByte(int address, byte myByte) {
  if (!active) {
    showNotActive();
    return;
  }

  if (address < this->size) {
    EEPROM.put(address, myByte);
  } else {
    showOutOfBoundsError(address);
  }
}

size_t MyEEPROM::readString(int address, char* value, size_t maxLen) {
  if (!active) {
    showNotActive();
    return 0;
  }

  if ((address + maxLen) < this->size) {
    return EEPROM.readString(address, value, maxLen);
  } else {
    showOutOfBoundsError(address, maxLen);
    return 0;
  }
}

String MyEEPROM::readString(int address) {
  if (!active) {
    showNotActive();
    return "";
  }

  if (address < this->size) {
    return EEPROM.readString(address);
  } else {
    showOutOfBoundsError(address);
    return "";
  }
}

size_t MyEEPROM::readBytes(int address, void* value, size_t maxLen) {
  
  if (!active) {
    showNotActive();
    return 0;
  }

  if ((address + maxLen) < this->size) {
    return EEPROM.readBytes(address, value, maxLen);
  } else {
    showOutOfBoundsError(address, maxLen);
    return 0;
  }
}

size_t MyEEPROM::writeString(int address, const char* value) {
  
  if (!active) {
    showNotActive();
    return 0;
  }

  if (address < this->size) {
    return EEPROM.writeString(address, value);
  } else {
    showOutOfBoundsError(address);
    return 0;
  }
}

size_t MyEEPROM::writeString(int address, String value) {

  if (!active) {
    showNotActive();
    return 0;
  }
  
  if (address < this->size) {
    return EEPROM.writeString(address, value);
  } else {
    showOutOfBoundsError(address);
    return 0;
  }  
}

size_t MyEEPROM::writeBytes(int address, const void* value, size_t len) {

  if (!active) {
    showNotActive();
    return 0;
  }

  if ((address + len) < this->size) {
    return EEPROM.writeBytes(address, value, len);
  } else {
    showOutOfBoundsError(address, len);
    return 0;
  }
}


#endif