#include <Arduino.h>
#include "defines.h"

#ifdef ENABLE_EEPROM  

#include "myeeprom.h"
#include <EEPROM.h>


#include "vars.h"

#include "debug.h"

MyEEPROM::MyEEPROM(int eepromSize) {
    debug->println(DEBUG_LEVEL_DEBUG, "[EEPROM]");
    this->size = eepromSize;
    active = false;
    debug->println(DEBUG_LEVEL_DEBUG, "[OK]");
};

bool MyEEPROM::start() {
  debug->print(DEBUG_LEVEL_INFO, "Starting EEPROM with ");
  debug->print(DEBUG_LEVEL_INFO, this->size);
  debug->println(DEBUG_LEVEL_INFO, " bytes... ");
  
  #ifdef ESP32  
    if (EEPROM.begin(this->size)) {
      debug->println(DEBUG_LEVEL_INFO, "Done");
      active = true;
    } else {
      debug->println(DEBUG_LEVEL_ERROR, "Failed");
      active = false;
    }    
  #endif

  #ifdef ESP8266
    EEPROM.begin(this->size);
    active = true;
  #endif
  return active;
}

void MyEEPROM::showOutOfBoundsError(int address, int size) {

  showOutOfBoundsError(address);
  debug->print(DEBUG_LEVEL_ERROR, "Out of bounds (size): ");
  debug->println(DEBUG_LEVEL_ERROR, size);
}

void MyEEPROM::showOutOfBoundsError(int address) {

  debug->print(DEBUG_LEVEL_ERROR, "Cannot access EEPROM at address ");
  debug->println(DEBUG_LEVEL_ERROR, address);  
}

void MyEEPROM::showNotActive() {
  debug->println(DEBUG_LEVEL_ERROR, "EEPROM module is not active. Aborting");
}

void MyEEPROM::createSignature() {
  debug->println(DEBUG_LEVEL_DEBUG, "Creating signature to EEPROM...");

  if (!active) {
    showNotActive();
    return;
  }

  EEPROM.write(0, 'T');
  EEPROM.write(1, 'C');
  if (EEPROM.commit()) {
    debug->println(DEBUG_LEVEL_DEBUG, "Done");
  } else {
    debug->println(DEBUG_LEVEL_ERROR, "Cannot write to EEPROM");
  }
}

bool MyEEPROM::hasSignature() {

  if (!active) {
    showNotActive();
    return false;
  }

  return ((EEPROM.read(0) == 'T') && (EEPROM.read(1) == 'C'));
}

void MyEEPROM::writeInt(int address, int number)
{ 
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
  EEPROM.commit();
}

int MyEEPROM::readInt(int address)
{
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
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
    EEPROM.commit();
  } else {
    showOutOfBoundsError(address);
  }
}

#ifdef ESP32
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
      int bytes = EEPROM.writeString(address, value);
      EEPROM.commit();
      return bytes;
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
      int bytes = EEPROM.writeString(address, value);
      EEPROM.commit();
      return bytes;
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
      int bytes = EEPROM.writeBytes(address, value, len);
      EEPROM.commit();
      return bytes;
    } else {
      showOutOfBoundsError(address, len);
      return 0;
    }
  }
#endif

#endif