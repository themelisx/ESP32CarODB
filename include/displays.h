#ifndef DISPLAYS_h
#define DISPLAYS_h

#include <Arduino.h>
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"
#include "defines.h"

class Displays {
  private:
    Adafruit_GC9A01A *tft;

    int screenWidth;
    int screenWidthCenter;
    int screenHeight;
    int screenHeightCenter;

  public:
    int activeView;
    int nextView;
    int secondaryActiveView;
    int count;

    Displays(Adafruit_GC9A01A *tft, int screenWidth, int screenHeight);
    
    int getScreenWidth();
    int getScreenHeight();

    Adafruit_GC9A01A *getTFT();

};

#endif