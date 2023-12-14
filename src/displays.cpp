#include <Arduino.h>

#include "displays.h"

#include "debug.h"
#include "vars.h"


Displays::Displays(Adafruit_GC9A01A *tft, int screenWidth, int screenHeight) {
    
    debug->println(DEBUG_LEVEL_DEBUG, "::Displays activated");

    this->tft = tft;
    this->screenHeight = screenHeight;
    this->screenHeightCenter = screenHeight / 2;
    this->screenWidth = screenWidth;    
    this->screenWidthCenter = screenWidth / 2;
    
    this->activeView = 0;
    this->nextView = 1;
    this->secondaryActiveView = 0;
    this->count = 0;
}

void Displays::printMsg(const char *buf) {

  int16_t x1, y1;
  uint16_t w, h;
  tft->fillScreen(BACK_COLOR);
  tft->getTextBounds(buf, screenWidthCenter, screenHeightCenter, &x1, &y1, &w, &h);
  tft->setCursor(screenWidthCenter - w / 2, screenHeightCenter + h / 2);
  tft->print(buf);
}

int Displays::getScreenWidth() {
    return this->screenWidth;
}

int Displays::getScreenHeight() {
    return this->screenHeight;
}

Adafruit_GC9A01A *Displays::getTFT() {
    return this->tft;
}
