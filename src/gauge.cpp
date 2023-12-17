#include <Arduino.h>

#include "gauge.h"

#include "defines.h"
#include "vars.h"
#include "debug.h"
#include "fonts.h"
#include "displays.h"

Gauge::Gauge(Displays *monitor, int id, int type, int interval, char *title, char *strFormat, int lowColor, int highColor, bool useLowWarning, bool useHighWarning, int min, int low, int high, int max) {

    debug->print(DEBUG_LEVEL_INFO, "Creating Gauge (title:'");
    debug->print(DEBUG_LEVEL_INFO, title);
    debug->print(DEBUG_LEVEL_INFO, "`, id:");
    debug->print(DEBUG_LEVEL_INFO, id);
    debug->print(DEBUG_LEVEL_INFO, ", type:");
    debug->print(DEBUG_LEVEL_INFO, type);
    debug->print(DEBUG_LEVEL_INFO, ", strFormat: `");
    debug->print(DEBUG_LEVEL_INFO, strFormat);
    debug->println(DEBUG_LEVEL_INFO, "')");

    #ifdef USE_MULTI_THREAD
      this->semaphore = xSemaphoreCreateMutex();
      xSemaphoreGive(this->semaphore);
    #endif

    this->monitor = monitor;
    this->id = id;
    this->type = type;    
    this->interval = interval;
    this->visible = true;
    this->secondaryViews.activeView = 0;
    this->secondaryViews.count = 0;

    this->data.min = min;
    this->data.low = low;
    this->data.high = high;
    this->data.max = max;
    this->data.value = INT_MIN;
    this->data.oldValue = INT_MIN;
    this->data.state = STATE_OUT_OF_RANGE;
    this->data.lowColor = lowColor;
    this->data.highColor = highColor;
    this->data.useLowWarning = useLowWarning;
    this->data.useHighWarning = useHighWarning;
    this->data.strFormat = strFormat;
    this->data.title = title;

    this->screenHeight = monitor->getScreenHeight();
    this->screenWidth = monitor->getScreenWidth();
    this->display = monitor->getTFT();

    outerRadius = (screenHeight / 2) - 1;
    innerRadius = outerRadius - 50;
    radius = innerRadius + 2;
    radiusLength = outerRadius - innerRadius - 2 - 2;
    angleStart = 150;
    angleEnd = 30;
    gaugeMin = 0;
    gaugeMax = 240;

    setFrontColor(FRONT_COLOR);
    setBackColor(BACK_COLOR);

    int angle = 0;
    int angle2 = angleStart;
    for (angle = 0; angle < 360; angle++) {
      
      x[angle] = (screenWidth / 2) + cos(angle2 * rad) * radius;
      y[angle] = (screenHeight / 2) + sin(angle2 * rad) * radius;
      x2[angle] = x[angle] + cos(angle2 * rad) * radiusLength;
      y2[angle] = y[angle] + sin(angle2 * rad) * radiusLength;
      angle2++;
      if (angle2 == 360) angle2 = 0;
    }
}

void Gauge::setFrontColor(int fColor) {
    this->fColor = fColor;
}

void Gauge::setBackColor(int bColor) {
    this->bColor = bColor;
}

void Gauge::setFontSize(int sz) {
  switch (sz) {
    //case 12: display->setFont(&Seven_Segment12pt7b); break;
    //case 14: display->setFont(&Seven_Segment14pt7b); break;
    /*case 16:
      display->setFont(&Seven_Segment16pt7b);
      break;  //default*/
    case 18: display->setFont(&Seven_Segment18pt7b); break;
    //case 20: display->setFont(&Seven_Segment20pt7b); break;
    //case 22: display->setFont(&Seven_Segment22pt7b); break;
    //case 24: display->setFont(&Seven_Segment24pt7b); break;
    case 26: display->setFont(&Seven_Segment26pt7b); break;
    //case 28: display->setFont(&Seven_Segment28pt7b); break;
    //case 30: display->setFont(&Seven_Segment30pt7b); break;
    //case 32: display->setFont(&Seven_Segment32pt7b); break;
    //case 34: display->setFont(&Seven_Segment34pt7b); break;
    //case 36: display->setFont(&Seven_Segment36pt7b); break;
    //case 38: display->setFont(&Seven_Segment38pt7b); break;
    //case 40: display->setFont(&Seven_Segment40pt7b); break;
    //case 42: display->setFont(&Seven_Segment42pt7b); break;
    //case 44: display->setFont(&Seven_Segment44pt7b); break;
    //case 46: display->setFont(&Seven_Segment46pt7b); break;
    default: display->setFont(&Seven_Segment18pt7b); break;
  }
}

void Gauge::getFormattedValue(int newValue, char *buf) {
  if (this->getId() == VIEW_BATTERY_VOLTAGE) {
    setFontSize(26);
    float x = (float)newValue / 10;
    sprintf(buf, data.strFormat, x);
  } else {
    setFontSize(26);
    sprintf(buf, data.strFormat, newValue);
  }
}


#ifdef ENABLE_RTC_CLOCK

void Gauge::drawDateTime() {
  strncpy(dateString, myRTC.getFormattedDate(), DATE_LENGTH);
  strncpy(timeString, myRTC.getFormattedTime(), TIME_LENGTH);

  if (strncmp(dateString, oldDateString, DATE_LENGTH) != 0) {
    strncpy(oldDateString, dateString, DATE_LENGTH);
    //setFontSize(16);
    setFontSize(18);
    drawBottomString(dateString, fColor, bColor);
  }
  if (strncmp(timeString, oldTimeString, TIME_LENGTH) != 0) {
    strncpy(oldTimeString, timeString, TIME_LENGTH);
    setFontSize(26);
    drawCenterString(timeString);
  }
}

#endif

void Gauge::drawGauge(bool repaint) {

  char valueBuf[16];
  char secondaryBuffer[16];
  int newState;
  int newStateColor;
  int fColorSecondary;
  int bColorSecondary;
  int secondaryValue;
  bool drawUpper = false;

  int newValue = this->data.value;

  if (newValue < data.min || newValue > data.max) {
    newState = STATE_OUT_OF_RANGE;
  } else {
    if (data.low != data.min && newValue >= data.min && newValue < data.low) {
      newState = STATE_LOW;
      newStateColor = data.lowColor;
    } else if (data.high != data.max && newValue > data.high && newValue <= data.max) {
      newState = STATE_HIGH;
      newStateColor = data.highColor;
    } else {
      newState = STATE_NORMAL;
      newStateColor = fColor;
    }
  }

  if (repaint) {
    setFontSize(18);
    drawBottomString(data.title, fColor, bColor);
  }

  //debug->println(newValue);

  if (newValue != INT_MIN) {
    display->setTextColor(newStateColor);
    getFormattedValue(newValue, valueBuf);
    drawCenterString(valueBuf);
  } else {
    display->setTextColor(fColor);
    drawCenterString("---");
  }

  if (getType() == TYPE_GAUGE_GRAPH) {
    if (secondaryViews.activeView == 0) {
      if (newState != STATE_OUT_OF_RANGE) {
        int i;

        int lowAngle = map(data.low, data.min, data.max, gaugeMin, gaugeMax);
        int highAngle = map(data.high, data.min, data.max, gaugeMin, gaugeMax);
        int oldAngle = map(data.oldValue, data.min, data.max, gaugeMin, gaugeMax);
        int newAngle = map(newValue, data.min, data.max, gaugeMin, gaugeMax);

        if (newValue < data.oldValue) {
          for (i = oldAngle; i >= newAngle; i--) {
            drawGaugeLine(i, bColor);
          }
          if (newState != data.state) {
            for (i = 0; i <= newAngle; i++) {
              drawGaugeLine(i, newStateColor);
            }
          }
        } else {
          if (newState != data.state) {
            oldAngle = 0;
          }
          for (i = oldAngle; i <= newAngle; i++) {
            drawGaugeLine(i, newStateColor);
          }
        }

        if (data.low != data.min && newValue < data.low) {
          drawGaugeLine(lowAngle, data.lowColor);
        }
        if (data.high != data.max && newValue < data.high) {
          drawGaugeLine(highAngle, data.highColor);
        }
      } else {
        debug->println(DEBUG_LEVEL_ERROR, "Out of range");        
      }
    } else {
      drawUpper = true;
    }
  } else if (getType() == TYPE_DUAL_TEXT) {
    drawUpper = true;
  }

  data.state = newState;
  data.oldValue = newValue;

  if (drawUpper && secondaryViews.activeView != 0) {
    
    int secondaryViewId = secondaryViews.ids[secondaryViews.activeView];
    newValue = secondaryViews.value[secondaryViews.activeView];
    if (newValue != secondaryViews.oldValue[secondaryViews.activeView]) {
      debug->print(DEBUG_LEVEL_DEBUG2, "Draw upper text: ");
      debug->println(DEBUG_LEVEL_DEBUG2, secondaryValue);
      secondaryViews.oldValue[secondaryViews.activeView] = newValue;
      
      if (newValue == INT_MIN) {
        sprintf(secondaryBuffer, "%s", "---");
      } else {
        if (secondaryViewId == VIEW_BATTERY_VOLTAGE) {
          float x = (float)newValue / 10;
          sprintf(secondaryBuffer, secondaryViews.strFormat[secondaryViews.activeView], x);
        } else {
          sprintf(secondaryBuffer, secondaryViews.strFormat[secondaryViews.activeView], newValue);
        }
      }

      drawUpperString(repaint, secondaryBuffer, FRONT_COLOR, BACK_COLOR);
    }   
  }    
}

void Gauge::drawGaugeLine(int angle, int color) {
  int angleBefore = angle - 1;
  int angleAfter = angle + 1;
  if (angleBefore < 0)
    angleBefore = angle - 1 + 359;
  if (angleAfter >= 359)
    angleAfter = angle + 1 - 359;

  display->fillTriangle(x[angleBefore], y[angleBefore], x2[angleBefore], y2[angleBefore], x2[angleAfter], y2[angleAfter], color);
  display->fillTriangle(x2[angleAfter], y2[angleAfter], x[angleBefore], y[angleBefore], x[angleAfter], y[angleAfter], color);
  
}

void Gauge::drawBorders() {
  display->drawCircle((screenWidth / 2), (screenHeight / 2), outerRadius, fColor);
  display->drawCircle((screenWidth / 2), (screenHeight / 2), innerRadius, fColor);

  display->drawLine(x[357], y[357], x2[357], y2[357], fColor);
  display->drawLine(x[gaugeMax + 2], y[gaugeMax + 2], x2[gaugeMax + 2], y2[gaugeMax + 2], fColor);

  display->fillTriangle(
    (screenWidth / 2) + 2,
    (screenHeight / 2) + 2,
    (screenWidth / 2) + cos((angleStart - 2) * rad) * screenWidth,
    (screenHeight / 2) + sin((angleStart - 2) * rad) * screenHeight,
    (screenWidth / 2) + cos((angleEnd + 2) * rad) * screenWidth,
    (screenHeight / 2) + sin((angleEnd + 2) * rad) * screenHeight,
    bColor);

  
}

void Gauge::drawCenterString(const char *buf) {
  int x = screenWidth / 2;
  int y = screenHeight / 2;
  int16_t x1, y1;
  uint16_t w, h;
  //char buf2[16];

  display->fillCircle((screenWidth / 2), (screenHeight / 2), innerRadius - 2, bColor);
  display->getTextBounds(buf, x, y, &x1, &y1, &w, &h);
  display->setCursor(x - w / 2, y + h / 2);
  display->print(buf);

  /*
  sprintf(buf2, "%s", "11:11");  
  display->getTextBounds(buf2, x, y, &x1, &y1, &w, &h);
  display->setCursor(x - w / 2, y + h / 2);
  display->print(buf2);
  debug->print(DEBUG_LEVEL_DEBUG2, ("w=");
  debug->println(DEBUG_LEVEL_DEBUG2, w);

  sprintf(buf2, "%s", "00:00");  
  display->getTextBounds(buf2, x, y, &x1, &y1, &w, &h);
  display->setCursor(x - w / 2, y + h / 2);
  display->print(buf2);
  debug->print(DEBUG_LEVEL_DEBUG2, "w=");
  debug->println(DEBUG_LEVEL_DEBUG2, w);
  */

  
}

void Gauge::drawUpperString(bool repaint, const char *buf, int fColor, int bgColor) {
  int16_t x1, y1;
  uint16_t w, h;
  int x = 120;
  int y = 34;

  setFontSize(18);
  display->setTextColor(fColor);

  display->getTextBounds(buf, x, y, &x1, &y1, &w, &h);
  
  display->fillRoundRect(48, -24, 144, 72, 20, bgColor);
  
  if (repaint) {
    display->drawRoundRect(47, -23, 146, 72, 20, fColor);
    
  }
  display->setCursor(x - w / 2, y + 5);
  display->print(buf);
}

void Gauge::drawBottomString(const char *buf, int fColor, int bgColor) {
  int16_t x1, y1;
  uint16_t w, h;
  int x = 120;
  int y = 220;

  display->setTextColor(fColor);

  display->getTextBounds(buf, x, y, &x1, &y1, &w, &h);
  
  display->fillRoundRect(48, 180, 144, 72, 20, bgColor);
  
  display->drawRoundRect(48, 188, 144, 72, 20, fColor);
  
  display->setCursor(x - w / 2, y + 5);
  display->print(buf);
  
}

void Gauge::addSecondaryView(int secondaryViewId, char *strFormat) {
    if (secondaryViews.count < MAX_SECONDARY_VIEWS) {
        int pos = secondaryViews.count + 1;
        secondaryViews.ids[pos] = secondaryViewId;
        secondaryViews.strFormat[pos] = strFormat;
        secondaryViews.value[pos] = INT_MIN;
        secondaryViews.oldValue[pos] = INT_MIN;
        secondaryViews.count++;
    } else {
        debug->println(DEBUG_LEVEL_ERROR, "Cannot add more views");
    }
    if (type == TYPE_DUAL_TEXT) {
        secondaryViews.activeView = 1;
    }
}

int Gauge::getViewHeight() {
    return monitor->getScreenHeight();
}

int Gauge::getViewWidth() {
    return monitor->getScreenWidth();
}

int Gauge::getId() {
    return id;
}

int Gauge::getType() {
    return type;
}

int Gauge::getInterval() {
    return interval;
}

bool Gauge::isVisible() {
    return visible;
}

