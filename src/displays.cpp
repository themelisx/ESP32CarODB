#include <Arduino.h>

#include "display.h"
#include "gauge.h"

#include "debug.h"
#include "vars.h"
#include "fonts.h"

Display::Display() {

}

Display::Display(int id, int8_t _CS, int8_t _DC, int screenWidth, int screenHeight) {
    
    debug->println(DEBUG_LEVEL_DEBUG, "::Display activated");

    tft = new Adafruit_GC9A01A(_CS, _DC);
    tft->begin();
    tft->setRotation(0);
    tft->fillScreen(BLACK);

    this->id = id;
    this->screenHeight = screenHeight;
    this->screenHeightCenter = screenHeight / 2;
    this->screenWidth = screenWidth;    
    this->screenWidthCenter = screenWidth / 2;
    
    this->activeViewIndex = 0;
    this->nextViewIndex = 1;
    this->secondaryActiveView = 0;
    this->count = 0;
    this->totalGauges = 0;
}

int Display::getId() {
  return id;
}

int Display::getTotalGauges() {
  return totalGauges;
}

bool Display::addGauge(int id, int type, int interval, char *title, char *strFormat, int lowColor, int highColor, bool useLowWarning, bool useHighWarning, int min, int low, int high, int max){
  if (totalGauges <= MAX_VIEWS) {
    totalGauges++;
    myGauges[totalGauges] = new Gauge(id, type, interval, title, strFormat, lowColor, highColor, useLowWarning, useHighWarning, min, low, high, max);
    return true;
  } else {
    debug->println(DEBUG_LEVEL_ERROR, "Cannot add more Gauges");
    return false;
  }
}

bool Display::addSecondaryView(int primaryView, int secondaryViewId, char *strFormat) {
  bool ret;
  for (int i=1; i<totalGauges; i++) {
    if (myGauges[i]->getId() == primaryView) {
      myGauges[i]->addSecondaryView(secondaryViewId, strFormat);
      ret = true;
      break;
    }
  }
  return ret;
}

Gauge* Display::getGauge(int gaugeId) {
  for (int i=1; i<totalGauges; i++) {
    if (myGauges[i]->getId() == gaugeId) {
      return myGauges[i];
    }
  }
  return nullptr;
}

void Display::printMsg(const char *buf) {

  int16_t x1, y1;
  uint16_t w, h;
  tft->setFont(&Seven_Segment18pt7b);
  tft->fillScreen(BACK_COLOR);
  tft->getTextBounds(buf, screenWidthCenter, screenHeightCenter, &x1, &y1, &w, &h);
  tft->setCursor(screenWidthCenter - w / 2, screenHeightCenter + h / 2);
  tft->print(buf);
}

int Display::getActiveViewIndex() {
  
  int ret;
  #ifdef USE_MULTI_THREAD
    xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
  #endif
  ret = this->activeViewIndex;
  #ifdef USE_MULTI_THREAD
    xSemaphoreGive(semaphoreActiveView);
  #endif
  return ret;

}

int Display::getActiveViewId() {
  
  int ret;
  #ifdef USE_MULTI_THREAD
    xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
  #endif
  ret = myGauges[activeViewIndex]->getId();
  #ifdef USE_MULTI_THREAD
    xSemaphoreGive(semaphoreActiveView);
  #endif
  return ret;

}

Gauge* Display::getActiveGauge() {
  
  Gauge *ret;
  #ifdef USE_MULTI_THREAD
    xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
  #endif
  ret = myGauges[activeViewIndex];
  #ifdef USE_MULTI_THREAD
    xSemaphoreGive(semaphoreActiveView);
  #endif
  return ret;

}

void Display::setActiveView(int newActiveView) {
 
  #ifdef USE_MULTI_THREAD
    xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
  #endif
  this->activeViewIndex = newActiveView;
  #ifdef USE_MULTI_THREAD
    xSemaphoreGive(semaphoreActiveView);
  #endif
}

void Display::setNextView(int newNextView) {
 
  #ifdef USE_MULTI_THREAD
    xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
  #endif
  this->nextViewIndex = newNextView;
  #ifdef USE_MULTI_THREAD
    xSemaphoreGive(semaphoreActiveView);
  #endif
}

int Display::getNextView() {
  int ret;
  #ifdef USE_MULTI_THREAD
    xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
  #endif
  ret = this->nextViewIndex;
  #ifdef USE_MULTI_THREAD
    xSemaphoreGive(semaphoreActiveView);
  #endif
  return ret;
}

int Display::getSecondaryActiveView() {
  
  int ret;
  #ifdef USE_MULTI_THREAD
    xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
  #endif
  ret = this->secondaryActiveView;
  #ifdef USE_MULTI_THREAD
    xSemaphoreGive(semaphoreActiveView);
  #endif
  return ret;

}

void Display::setSecondaryActiveView(int newSecondaryActiveView) {
 
  #ifdef USE_MULTI_THREAD
    xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
  #endif
  this->secondaryActiveView = newSecondaryActiveView;
  #ifdef USE_MULTI_THREAD
    xSemaphoreGive(semaphoreActiveView);
  #endif
}

int Display::getScreenWidth() {
    return this->screenWidth;
}

int Display::getScreenHeight() {
    return this->screenHeight;
}

/*Adafruit_GC9A01A* Display::getTFT() {
    return this->tft;
}*/

void Display::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  tft->drawCircle(x0, y0, r, color);
}

void Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  tft->drawLine(x0, y0, x1, y1, color);
}

void Display::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
  tft->fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

void Display::setTextColor(uint16_t c) {
  tft->setTextColor(c);
}

void Display::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  tft->fillCircle(x0, y0, r, color);
}

void Display::getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
  tft->getTextBounds(string,  x, y, x1, y1, w, h);
}

void Display::setCursor(int16_t x, int16_t y) {
  tft->setCursor(x, y);
}

void Display::fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) {
  tft->fillRoundRect(x0, y0, w, h, radius, color);
}

void Display::drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) {
  tft->drawRoundRect(x0, y0, w, h, radius, color);
}

void Display::print(const char *str) {
  tft->print(str);
}

void Display::setFont(const GFXfont *f) {
  tft->setFont(f);
}

void Display::fillScreen(uint16_t color) {
  tft->fillScreen(color);
}