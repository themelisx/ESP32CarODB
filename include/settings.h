#ifndef SETTINGS_h
#define SETTINGS_h

#include <Arduino.h>

class Settings {
  private:
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