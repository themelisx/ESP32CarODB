#include <Arduino.h>

#include "displayManager.h"

#include "debug.h"
#include "vars.h"
#include "fonts.h"
#include "display.h"
#include "gauge.h"


DisplayManager::DisplayManager() {
    activeDisplay = 0;
    count = 0;
}

bool DisplayManager::addDisplay(Adafruit_GC9A01A* monitor, int displayID, int screenWidth, int screenHeight) {
    if (count < MAX_DISPLAYS) {
        count++;
        debug->println(DEBUG_LEVEL_INFO, "Staring up display...");
        myDisplays[count] = new Display(monitor, displayID, screenWidth, screenHeight);
        return true;
    } else {
        debug->println(DEBUG_LEVEL_ERROR, "Cannot add more display");
        return false;
    }
}

void DisplayManager::goToPreviousDisplay() {
    #ifdef USE_MULTI_THREAD
        xSemaphoreTake(semaphoreActiveDisplay, portMAX_DELAY);
    #endif

    activeDisplay--;
    if (activeDisplay < 1) {
        activeDisplay = count;
    }

    #ifdef USE_MULTI_THREAD
        xSemaphoreGive(semaphoreActiveDisplay);
        yield();
    #endif
}
void DisplayManager::goToNextDisplay() {
    #ifdef USE_MULTI_THREAD
        xSemaphoreTake(semaphoreActiveDisplay, portMAX_DELAY);
    #endif

    activeDisplay++;
    if (activeDisplay > count) {
    activeDisplay = 1;
    }

    #ifdef USE_MULTI_THREAD
        xSemaphoreGive(semaphoreActiveDisplay);
        yield();
    #endif
}

void DisplayManager::goToPreviousView() {
    
    bool changeGauge = true;
    #ifdef USE_MULTI_THREAD
        xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
    #endif

    Display *display = myDisplays[activeDisplay];
    Gauge *gauge = display->getActiveGauge();

    if (gauge->secondaryViews.count > 0) {
    gauge->secondaryViews.activeViewIndex--;

    if (gauge->secondaryViews.activeViewIndex > 0) {
        debug->print(DEBUG_LEVEL_INFO, "Changing to prev secondary view");
        changeGauge = false;
    } else {
        gauge->secondaryViews.activeViewIndex = gauge->secondaryViews.count;
    }
    }

    if (changeGauge) {
        display->setActiveView(display->getActiveViewIndex() - 1);
        if (display->getNextView() == 0) {
            display->setNextView(display->getTotalGauges());
        }
        /*if (display->nextView == VIEW_DATE_TIME) {
            memset(oldDateString, 0, DATE_LENGTH);
            memset(oldTimeString, 0, TIME_LENGTH);
        }*/
    }
    display->getActiveGauge()->setRepaint(true);
    
    mySettings->setActiveView(display->getId(), display->getActiveViewId());
    mySettings->setSecondaryActiveView(display->getId(), display->getSecondaryActiveView());
    mySettings->save();      
    
    #ifdef USE_MULTI_THREAD        
        xSemaphoreGive(semaphoreActiveView);
        yield();
    #endif

    
}
void DisplayManager::goToNextView() {
    bool changeGauge = true;
    
    #ifdef USE_MULTI_THREAD
    xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
    #endif

    Display *display = myDisplays[activeDisplay];
    Gauge *gauge = display->getActiveGauge();

    if (gauge->secondaryViews.count > 0) {
    gauge->secondaryViews.activeViewIndex++;

    if (gauge->secondaryViews.activeViewIndex <= gauge->secondaryViews.count) {
        debug->println(DEBUG_LEVEL_INFO, "Changing to next secondary view");
        changeGauge = false;
    } else {
        debug->println(DEBUG_LEVEL_INFO, "Changing to next view");
        if (gauge->getType() == TYPE_DUAL_TEXT) {
        debug->println(DEBUG_LEVEL_DEBUG, "view is dual text");
        gauge->secondaryViews.activeViewIndex = 1;
        } else {
        debug->println(DEBUG_LEVEL_DEBUG, "view is NOT dual text");
        gauge->secondaryViews.activeViewIndex = 0;
        }
    }
    }

    if (changeGauge) {
        display->setNextView(display->getActiveViewIndex() + 1);
        if (display->getNextView() > display->getTotalGauges()) {
            display->setNextView(1);
        }
        /*if (display->nextView == VIEW_DATE_TIME) {
            memset(oldDateString, 0, DATE_LENGTH);
            memset(oldTimeString, 0, TIME_LENGTH);
        }*/
    }

    display->getActiveGauge()->setRepaint(true);

    mySettings->setActiveView(display->getId(), display->getActiveViewId());
    mySettings->setSecondaryActiveView(display->getId(), display->getSecondaryActiveView());
    mySettings->save();
    
    #ifdef USE_MULTI_THREAD    
        xSemaphoreGive(semaphoreActiveView);
        yield();
    #endif
}

int DisplayManager::getActiveDisplayId() {

  int ret;
  #ifdef USE_MULTI_THREAD
    xSemaphoreTake(semaphoreActiveDisplay, portMAX_DELAY);
  #endif
  ret = myDisplays[activeDisplay]->getId();
  #ifdef USE_MULTI_THREAD
    xSemaphoreGive(semaphoreActiveDisplay);
  #endif
  return ret;
}

void DisplayManager::setActiveDisplay(int activeDisplay) {

  #ifdef USE_MULTI_THREAD
    xSemaphoreTake(semaphoreActiveDisplay, portMAX_DELAY);
  #endif
  this->activeDisplay = activeDisplay;
  #ifdef USE_MULTI_THREAD
    xSemaphoreGive(semaphoreActiveDisplay);
  #endif
}

Display *DisplayManager::getDisplay(int displayID) {
    for (int i=1; i<= MAX_DISPLAYS; i++) {
        if (displayID == myDisplays[i]->getId()) {
            return myDisplays[i];
        }
    }
    debug->println(DEBUG_LEVEL_ERROR, "This Display ID does not exists");
    return nullptr;    
}

#ifdef ENABLE_STARTUP_LOGO
void DisplayManager::showStartupLogo(int displayID) {
    if (displayID == 1) {
        debug->println(DEBUG_LEVEL_INFO, "Drawing startup logo at TFT1...");
        tft1.drawBitmap(0, 0, epd_logo1, SCREEN_H, SCREEN_W, WHITE);
        yield();
    } else if (displayID == 2) {
        #ifdef ENABLE_SECOND_DISPLAY
            debug->println(DEBUG_LEVEL_INFO, "Drawing startup logo at TFT2...");
            tft2.drawBitmap(0, 0, epd_logo2, SCREEN_H, SCREEN_W, WHITE);
            yield();
        #else
            debug->println(DEBUG_LEVEL_ERROR, "Secondary TFT is disabled by the configuration");
        #endif
    } else {
        debug->println(DEBUG_LEVEL_ERROR, "Unknown Display ID");
    }    
}
#endif