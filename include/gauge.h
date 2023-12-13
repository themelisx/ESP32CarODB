#ifndef GAUGES_h
#define GAUGES_h

#include <Arduino.h>
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"
#include "defines.h"
#include "displays.h"

#define STATE_UNKNOWN -1
#define STATE_LOW 0
#define STATE_NORMAL 1
#define STATE_HIGH 2
#define STATE_OUT_OF_RANGE 3

#define rad 0.01745

#define MAX_VIEWS 7
#define MAX_SECONDARY_VIEWS 4

typedef struct
{
  int activeView;
  int count;
  int ids[MAX_SECONDARY_VIEWS + 1];
  char *strFormat[MAX_SECONDARY_VIEWS + 1];
} S_SecondaryViews;

typedef struct
{
  int min;
  int low;
  int high;
  int max;
  int value;
  int state;
  int lowColor;
  int highColor;
  bool useLowWarning;
  bool useHighWarning;
  char *strFormat;
  char *title;
} S_Gauge;

class Gauge {
  private:
    int fColor;
    int bColor;

    int screenHeight;
    int screenWidth;
    
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

    char dateString[DATE_LENGTH];
    char timeString[TIME_LENGTH];
    char oldDateString[DATE_LENGTH];
    char oldTimeString[TIME_LENGTH];

    int activeDisplay;

    Adafruit_GC9A01A *display;
    
    Displays *monitor;
    int id;
    int type;
    int interval;
    bool visible;
    
    Gauge *gauge;

  public:
    Gauge(Displays *monitor, int id, int type, int interval, char *title, char *strFormat, int lowColor, int highColor, bool useLowWarning, bool useHighWarning, int min, int low, int high, int max);

    S_Gauge data;
    S_SecondaryViews secondaryViews;
    
    #ifdef USE_MULTI_THREAD
    SemaphoreHandle_t semaphore;
    #endif
    
    void addSecondaryView(int id, int secondaryViewId, char *strFormat);

    int getViewHeight();
    int getViewWidth();

    int getId();
    int getType();
    int getInterval();
    bool isVisible();

    void setFrontColor(int fColor);
    void setBackColor(int bColor);    
    void setFontSize(int sz);
    void getFormattedValue(int viewId, int newValue, char *buf);
    void drawDateTime();
    void drawGauge(int viewId, bool repaint, int newValue);
    void drawGaugeLine(int angle, int color);
    void drawBorders();
    void drawCenterString(const char *buf);
    void drawUpperString(bool repaint, const char *buf, int fColor, int bgColor);
    void drawBottomString(const char *buf, int fColor, int bgColor);

    int getSecondaryInfo(int viewId, char *buf);

};

#endif
