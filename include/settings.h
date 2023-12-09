#ifndef SETTINGS_h
#define SETTINGS_h

#include <Arduino.h>

class Settings {
  private:
    bool status;
    byte debugLevel;

  public:
    Settings();

    void load();
    void save();
    void setDefaults();
};

#endif