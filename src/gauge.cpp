#include <Arduino.h>

#include "defines.h"
#include "vars.h"
#include "debug.h"
#include "fonts.h"
#include "display.h"
#include "gauge.h"

Gauge::Gauge(int id, int type, int interval, char *title, char *strFormat, int lowColor, int highColor, bool useLowWarning, bool useHighWarning, int min, int low, int high, int max) {

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

    this->id = id;
    this->type = type;    
    this->interval = interval;
    this->visible = true;
    this->secondaryViews.activeViewIndex = 0;
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

    this->screenHeight = Display::getScreenHeight();
    this->halfScreenHeight = this->screenHeight / 2;

    this->screenWidth = Display::getScreenWidth();
    this->halfScreenWidth = this->screenWidth / 2;

    outerRadius = halfScreenHeight - 1;
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
      
      x[angle] = halfScreenWidth + cos(angle2 * rad) * radius;
      y[angle] = halfScreenHeight + sin(angle2 * rad) * radius;
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
    //case 12: Display::setFont(&Seven_Segment12pt7b); break;
    //case 14: Display::setFont(&Seven_Segment14pt7b); break;
    /*case 16:
      Display::setFont(&Seven_Segment16pt7b);
      break;  //default*/
    case 18: Display::setFont(&Seven_Segment18pt7b); break;
    //case 20: Display::setFont(&Seven_Segment20pt7b); break;
    //case 22: Display::setFont(&Seven_Segment22pt7b); break;
    //case 24: Display::setFont(&Seven_Segment24pt7b); break;
    case 26: Display::setFont(&Seven_Segment26pt7b); break;
    //case 28: Display::setFont(&Seven_Segment28pt7b); break;
    //case 30: Display::setFont(&Seven_Segment30pt7b); break;
    //case 32: Display::setFont(&Seven_Segment32pt7b); break;
    //case 34: Display::setFont(&Seven_Segment34pt7b); break;
    //case 36: Display::setFont(&Seven_Segment36pt7b); break;
    //case 38: Display::setFont(&Seven_Segment38pt7b); break;
    //case 40: Display::setFont(&Seven_Segment40pt7b); break;
    //case 42: Display::setFont(&Seven_Segment42pt7b); break;
    //case 44: Display::setFont(&Seven_Segment44pt7b); break;
    //case 46: Display::setFont(&Seven_Segment46pt7b); break;
    default: Display::setFont(&Seven_Segment18pt7b); break;
  }
}

void Gauge::getFormattedValue(int newValue, char *buf) {
  //char tmpBuf[16];

  if (this->getId() == VIEW_BATTERY_VOLTAGE) {
    setFontSize(26);
    float x = (float)newValue / 10;
    sprintf(buf, data.strFormat, x);
  } else {
    setFontSize(26);
    sprintf(buf, data.strFormat, newValue);    
  }
  
  /*
  // Add space before and after char '1'
	int i = 0;
  int j = 0;
	while (tmpBuf[i]) {
    if (tmpBuf[i] == '1') {
      if (i > 0) {
        buf[j] = ' '; j++;
      }
      buf[j] = '1'; j++;
      if (tmpBuf[i+1] != 0) {
        buf[j] = ' ';
      }
    } else {
      buf[j] = tmpBuf[i];
    }
    j++;
		i++;
	}
  */
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
    drawCenterString(timeString, true);
  }
}

#endif

bool Gauge::valueHasChanged() {

  bool ret = false;

  #ifdef USE_MULTI_THREAD
  xSemaphoreTake(semaphoreData, portMAX_DELAY);
  xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
  #endif

  if (data.value != data.oldValue) {
    ret = true;
  } else {    
    if (secondaryViews.activeViewIndex != VIEW_NONE) {
      int secondaryViewIdx = secondaryViews.activeViewIndex;
      //int secondaryViewId = secondaryViews.ids[secondaryViewIdx];      
      if (secondaryViews.value[secondaryViewIdx] != secondaryViews.oldValue[secondaryViewIdx]) {
        ret = true;
      }
    }
  }
  #ifdef USE_MULTI_THREAD
  xSemaphoreGive(semaphoreData);
  xSemaphoreGive(semaphoreActiveView);
  #endif
  return ret;
}

void Gauge::draw(bool repaint) {

  char valueBuf[16];
  char secondaryBuffer[16];
  int newState;
  int newStateColor;
  //int fColorSecondary;
  //int bColorSecondary;
  //int secondaryValue;
  bool drawUpper = false;

  #ifdef USE_MULTI_THREAD
  xSemaphoreTake(semaphoreData, portMAX_DELAY);
  #endif
  int newValue = this->data.value;
  #ifdef USE_MULTI_THREAD
  xSemaphoreGive(semaphoreData);
  #endif

  newStateColor = WHITE;

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

  // Clear old text  
  Display::setTextColor(BACK_COLOR);
  if (data.oldValue != INT_MIN) {            
    getFormattedValue(data.oldValue, valueBuf);
    drawCenterString(valueBuf, false);
  } else {
    drawCenterString("---", false);
  }

  // draw new text
  if (newValue != INT_MIN) {        
    Display::setTextColor(newStateColor);
    getFormattedValue(newValue, valueBuf);
    drawCenterString(valueBuf, false);
  } else {
    Display::setTextColor(fColor);
    drawCenterString("---", false);
  }

  #ifdef USE_MULTI_THREAD
  xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
  #endif
  int secondaryViewsActiveView = secondaryViews.activeViewIndex;
  #ifdef USE_MULTI_THREAD
  xSemaphoreGive(semaphoreActiveView);
  #endif

  if (getType() == TYPE_GAUGE_GRAPH) {
    if (secondaryViewsActiveView == 0) {
      if (newState != STATE_OUT_OF_RANGE) {

        #ifdef USE_MULTI_THREAD
        xSemaphoreTake(semaphoreData, portMAX_DELAY);
        #endif

        int oldValue = data.oldValue;

        #ifdef USE_MULTI_THREAD
        xSemaphoreGive(semaphoreData);
        #endif

        #ifndef DRAW_FAST
          int lowAngle = map(data.low, data.min, data.max, gaugeMin, gaugeMax);
          int highAngle = map(data.high, data.min, data.max, gaugeMin, gaugeMax);
        #endif
        int oldAngle = map(oldValue, data.min, data.max, gaugeMin, gaugeMax);
        int newAngle = map(newValue, data.min, data.max, gaugeMin, gaugeMax);

        if (newValue < oldValue) {
          #ifdef DRAW_FAST
            drawGaugeLine(oldAngle, BACK_COLOR);
            drawGaugeLine(newAngle, newStateColor);
          #else
            for (int i = oldAngle; i >= newAngle; i--) {
              drawGaugeLine(i, bColor);
            }
            if (newState != data.state) {
              for (int i = 0; i <= newAngle; i++) {
                drawGaugeLine(i, newStateColor);
              }
            }
          #endif
        } else {          
          if (newState != data.state) {
            oldAngle = 0;
          }
          #ifdef DRAW_FAST
            drawGaugeLine(oldAngle, BACK_COLOR);
            drawGaugeLine(newAngle, newStateColor);
          #else            
            for (int i = oldAngle; i <= newAngle; i++) {
              drawGaugeLine(i, newStateColor);
            }
          #endif
        }

        #ifndef DRAW_FAST
          if (data.low != data.min && newValue < data.low) {
            drawGaugeLine(lowAngle, data.lowColor);
          }
          if (data.high != data.max && newValue < data.high) {
            drawGaugeLine(highAngle, data.highColor);
          }
        #endif
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

  #ifdef USE_MULTI_THREAD
  xSemaphoreTake(semaphoreData, portMAX_DELAY);
  #endif

  data.oldValue = newValue;

  #ifdef USE_MULTI_THREAD
  xSemaphoreGive(semaphoreData);
  #endif

  if (drawUpper && secondaryViewsActiveView != 0) {
    
    int secondaryViewId = secondaryViews.ids[secondaryViewsActiveView];
    newValue = secondaryViews.value[secondaryViewsActiveView];
    int oldValue = secondaryViews.oldValue[secondaryViewsActiveView];
    if (newValue != oldValue) {
      debug->print(DEBUG_LEVEL_DEBUG2, "Draw upper text: ");
      debug->println(DEBUG_LEVEL_DEBUG2, newValue);
      secondaryViews.oldValue[secondaryViewsActiveView] = newValue;
      
      // Clear old text
      if (oldValue == INT_MIN) {
        sprintf(secondaryBuffer, "%s", "---");
      } else {
        if (secondaryViewId == VIEW_BATTERY_VOLTAGE) {
          float x = (float)oldValue / 10;
          sprintf(secondaryBuffer, secondaryViews.strFormat[secondaryViewsActiveView], x);
        } else {
          sprintf(secondaryBuffer, secondaryViews.strFormat[secondaryViewsActiveView], oldValue);
        }
      }
      drawUpperString(repaint, secondaryBuffer, BACK_COLOR, BACK_COLOR);
      
      // draw new text
      if (newValue == INT_MIN) {
        sprintf(secondaryBuffer, "%s", "---");
      } else {
        if (secondaryViewId == VIEW_BATTERY_VOLTAGE) {
          float x = (float)newValue / 10;
          sprintf(secondaryBuffer, secondaryViews.strFormat[secondaryViewsActiveView], x);
        } else {
          sprintf(secondaryBuffer, secondaryViews.strFormat[secondaryViewsActiveView], newValue);
        }
      }

      drawUpperString(repaint, secondaryBuffer, FRONT_COLOR, BACK_COLOR);
      //drawUpperString(true, secondaryBuffer, FRONT_COLOR, BACK_COLOR);
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

  Display::fillTriangle(x[angleBefore], y[angleBefore], x2[angleBefore], y2[angleBefore], x2[angleAfter], y2[angleAfter], color);
  Display::fillTriangle(x2[angleAfter], y2[angleAfter], x[angleBefore], y[angleBefore], x[angleAfter], y[angleAfter], color);
  
}

void Gauge::drawBorders() {
  Display::drawCircle(halfScreenWidth, halfScreenHeight, outerRadius, fColor);
  Display::drawCircle(halfScreenWidth, halfScreenHeight, innerRadius, fColor);

  Display::drawLine(x[357], y[357], x2[357], y2[357], fColor);
  Display::drawLine(x[gaugeMax + 2], y[gaugeMax + 2], x2[gaugeMax + 2], y2[gaugeMax + 2], fColor);

  Display::fillTriangle(
    halfScreenWidth + 2,
    halfScreenHeight + 2,
    halfScreenWidth + cos((angleStart - 2) * rad) * screenWidth,
    halfScreenHeight + sin((angleStart - 2) * rad) * screenHeight,
    halfScreenWidth + cos((angleEnd + 2) * rad) * screenWidth,
    halfScreenHeight + sin((angleEnd + 2) * rad) * screenHeight,
    bColor);
}

void Gauge::drawCenterString(const char *buf, bool clearCircleArea) {
  int x = halfScreenWidth;
  int y = halfScreenHeight;
  int16_t x1, y1;
  uint16_t w, h;
  //char buf2[16];

  if (clearCircleArea) {
    Display::fillCircle(halfScreenWidth, halfScreenHeight, innerRadius - 2, bColor);
  }
  Display::getTextBounds(buf, x, y, &x1, &y1, &w, &h);
  Display::setCursor(x - w / 2, y + h / 2);
  Display::print(buf);
 
}

void Gauge::drawUpperString(bool repaint, const char *buf, int fColor, int bgColor) {
  int16_t x1, y1;
  uint16_t w, h;
  int x = 120;
  int y = 34;

  setFontSize(18);
  Display::setTextColor(fColor);
  Display::getTextBounds(buf, x, y, &x1, &y1, &w, &h);  
  
  if (repaint) {
    Display::fillRoundRect(48, -24, 144, 72, 20, bgColor);
    Display::drawRoundRect(47, -23, 146, 72, 20, fColor);    
  }
  Display::setCursor(x - w / 2, y + 5);
  Display::print(buf);
}

void Gauge::drawBottomString(const char *buf, int fColor, int bgColor) {
  int16_t x1, y1;
  uint16_t w, h;
  int x = 120;
  int y = 220;

  Display::setTextColor(fColor);

  Display::getTextBounds(buf, x, y, &x1, &y1, &w, &h);
  
  Display::fillRoundRect(48, 180, 144, 72, 20, bgColor);
  
  Display::drawRoundRect(48, 188, 144, 72, 20, fColor);
  
  Display::setCursor(x - w / 2, y + 5);
  Display::print(buf);
  
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
        secondaryViews.activeViewIndex = 1;
    }
}

int Gauge::getViewHeight() {
    return Display::getScreenHeight();
}

int Gauge::getViewWidth() {
    return Display::getScreenWidth();
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

