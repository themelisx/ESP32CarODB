#ifndef GAUGES_h
#define GAUGES_h

#include <Arduino.h>
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"
#include "defines.h"
#include "display.h"

#define STATE_UNKNOWN -1
#define STATE_LOW 0
#define STATE_NORMAL 1
#define STATE_HIGH 2
#define STATE_OUT_OF_RANGE 3

#define rad 0.01745

typedef struct
{
  int activeViewIndex;
  int count;
  int ids[MAX_SECONDARY_VIEWS + 1];
  int value[MAX_SECONDARY_VIEWS + 1];
  int oldValue[MAX_SECONDARY_VIEWS + 1];
  char *strFormat[MAX_SECONDARY_VIEWS + 1];
} S_SecondaryViews;

typedef struct
{
  int min;
  int low;
  int high;
  int max;
  int value;
  int oldValue;
  int state;
  int lowColor;
  int highColor;
  bool useLowWarning;
  bool useHighWarning;
  char *strFormat;
  char *title;
} S_Gauge;

class Gauge: public Display {
  private:
    int fColor;
    int bColor;

    int screenHeight;
    int screenWidth;

    int halfScreenHeight;
    int halfScreenWidth;
    
    int outerRadius;
    int innerRadius;
    int radius;
    int radiusLength;
    int angleStart;
    int angleEnd;
    int gaugeMin;
    int gaugeMax;

    float x[360];
    float y[360];
    float x2[360];
    float y2[360];

    
    #ifdef ENABLE_RTC_CLOCK
      char dateString[DATE_LENGTH];
      char timeString[TIME_LENGTH];
      char oldDateString[DATE_LENGTH];
      char oldTimeString[TIME_LENGTH];
    #endif

    int activeDisplay;

    int id;
    int type;
    int interval;
    bool visible;
    
    Gauge *gauge;

    void drawGaugeLine(int angle, int color);
    void drawCenterString(const char *buf, bool clearCircleArea);
    void drawUpperString(bool repaint, const char *buf, int fColor, int bgColor);
    void drawBottomString(const char *buf, int fColor, int bgColor);
    void getFormattedValue(int newValue, char *buf);
    void drawDateTime();

  public:
    Gauge(int id, int type, int interval, char *title, char *strFormat, int lowColor, int highColor, bool useLowWarning, bool useHighWarning, int min, int low, int high, int max);

    S_Gauge data;
    S_SecondaryViews secondaryViews;
    
    #ifdef USE_MULTI_THREAD
    SemaphoreHandle_t semaphore;
    #endif
    
    void addSecondaryView(int secondaryViewId, char *strFormat);

    int getViewHeight();
    int getViewWidth();

    int getId();
    int getType();
    int getInterval();
    bool isVisible();

    void setFrontColor(int fColor);
    void setBackColor(int bColor);    
    void setFontSize(int sz);
    
    void draw(bool repaint);    
    void drawBorders();
    bool valueHasChanged();

};

#endif
