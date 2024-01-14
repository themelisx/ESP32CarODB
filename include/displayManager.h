#ifndef DISPLAY_MANAGER_h
#define DISPLAY_MANAGER_h

#include <Arduino.h>
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"
#include "defines.h"
#include "display.h"

class DisplayManager {
  private:

    Display *myDisplays[MAX_DISPLAYS + 1];

    int activeDisplay;
    int count;

  public:
    DisplayManager();

    bool addDisplay(Adafruit_GC9A01A* monitor, int displayID, int screenWidth, int screenHeight);
    Display *getDisplay(int DisplayID);

    int getActiveDisplayId();
    void setActiveDisplay(int activeDisplay);

    void goToPreviousDisplay();
    void goToNextDisplay();

    void goToPreviousView();
    void goToNextView();

    #ifdef ENABLE_STARTUP_LOGO
        void showStartupLogo(int displayID);
    #endif

};

#endif