#ifndef MY_EEPROM_h
#define MY_EEPROM_h

#include <Arduino.h>

class MyEEPROM {
  private:
    bool active;
    int size;

    void showOutOfBoundsError(int address, int size);
    void showOutOfBoundsError(int address);
    void showNotActive();

  public:
    MyEEPROM(int eepromSize);
    bool start();

    void createSignature();
    bool hasSignature();

    void writeInt(int address, int number);
    int readInt(int address);

    byte readByte(int address);
    void writeByte(int address, byte myByte);

    #ifdef ESP32
      size_t readString(int address, char* value, size_t maxLen);    
      size_t writeString(int address, const char* value);

      String readString(int address);
      size_t writeString(int address, String value);

      size_t readBytes(int address, void* value, size_t maxLen);
      size_t writeBytes(int address, const void* value, size_t len);
    #endif

};

#endif