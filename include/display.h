#ifndef DISPLAYS_h
#define DISPLAYS_h

#include <Arduino.h>
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"
#include "defines.h"

class Gauge;

class Display {
  private:
    Adafruit_GC9A01A *tft;

    Gauge *myGauges[MAX_VIEWS + 1];

    int id;

    int screenWidth;
    int screenWidthCenter;
    int screenHeight;
    int screenHeightCenter;

    int totalGauges;

    int activeViewIndex;
    int nextViewIndex;
    int secondaryActiveView;
    int count;

  public:

    Display();
    Display(int id, int8_t _CS, int8_t _DC, int screenWidth, int screenHeight);

    int getId();
    int getTotalGauges();

    bool addGauge(int id, int type, int interval, char *title, char *strFormat, int lowColor, int highColor, bool useLowWarning, bool useHighWarning, int min, int low, int high, int max);
    bool addSecondaryView(int primaryView, int secondaryViewId, char *strFormat);
    Gauge* getGauge(int gaugeId);
    
    int getScreenWidth();
    int getScreenHeight();
    void printMsg(const char *buf);

    int getActiveDisplayId();
    int getActiveViewIndex();
    int getActiveViewId();
    Gauge* getActiveGauge();
    int getSecondaryActiveView();

    void setActiveView(int newActiveView);

    int getNextView();
    void setNextView(int newNextView);

    void setSecondaryActiveView(int newSecondaryActiveView);

    Adafruit_GC9A01A* getTFT();
};

#endif