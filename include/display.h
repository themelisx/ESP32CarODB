#ifndef DISPLAYS_h
#define DISPLAYS_h

#include <Arduino.h>
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"
#include "defines.h"

class Gauge;

class Display {
  private:
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

    Gauge *myGauges[MAX_VIEWS + 1];

  public:

    Display();
    Display(int id, int8_t _CS, int8_t _DC, int screenWidth, int screenHeight);

    void updateDisplay();
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

    //Adafruit_GC9A01A* getTFT();

    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void setTextColor(uint16_t c);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void setCursor(int16_t x, int16_t y);
    void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
    void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
    void print(const char *str);
    void setFont(const GFXfont *f);
    void fillScreen(uint16_t color);
};

#endif