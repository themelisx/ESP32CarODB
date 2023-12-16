#include <Arduino.h>

#include "defines.h"

#ifdef USE_MULTI_THREAD

#include "debug.h"
#include "gauge.h"
#include "vars.h"
#include "displays.h"

void keypad_task(void *pvParameters) {
  debug->print(DEBUG_LEVEL_INFO, "Keypad manager task running on core ");
  debug->println(DEBUG_LEVEL_INFO, xPortGetCoreID());

  bool testDownKey = true;

  for (;;) {
    
    if (digitalRead(PIN_LEFT_KEY) == LOW) { // LEFT KEY PRESSED

      debug->println(DEBUG_LEVEL_DEBUG, "Left key pressed");
      
      xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);
      activeDisplay--;
      if (activeDisplay < 1) {
        activeDisplay = MAX_DISPLAYS;
      }
      xSemaphoreGive(keyPadSemaphore);
      yield();

    } else if (digitalRead(PIN_RIGHT_KEY) == LOW) {

      debug->println(DEBUG_LEVEL_DEBUG, "Right key pressed");
      
      xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);
      activeDisplay++;
      if (activeDisplay > MAX_DISPLAYS) {
        activeDisplay = 1;
      }
      xSemaphoreGive(keyPadSemaphore);
      yield();

    } else if (digitalRead(PIN_ENTER_KEY) == LOW) { // ENTER KEY PRESSED

      debug->println(DEBUG_LEVEL_DEBUG, "Enter key pressed");
      
    } else if (digitalRead(PIN_UP_KEY) == LOW) { // UP KEY PRESSED

      debug->println(DEBUG_LEVEL_DEBUG, "Up key pressed");

      bool changeGauge = true;
      xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);

      if (myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.count > 0) {
        myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.activeView--;

        if (myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.activeView > 0) {
          debug->print(DEBUG_LEVEL_INFO, "Changing to prev secondary view");
          changeGauge = false;
        } else {
          myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.activeView = myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.count;
        }
      }

      if (changeGauge) {
        myDisplays[activeDisplay]->nextView = myDisplays[activeDisplay]->activeView - 1;
        if (myDisplays[activeDisplay]->nextView == 0) {
          myDisplays[activeDisplay]->nextView = MAX_VIEWS;
        }
        /*if (myDisplays[activeDisplay]->nextView == VIEW_DATE_TIME) {
          memset(oldDateString, 0, DATE_LENGTH);
          memset(oldTimeString, 0, TIME_LENGTH);
        }*/
      }
      mySettings.save();
      xSemaphoreGive(keyPadSemaphore);
      yield();

    } else if (digitalRead(PIN_DOWN_KEY) == LOW || testDownKey) { // DOWN KEY PRESSED

      bool changeGauge = true;

      debug->println(DEBUG_LEVEL_DEBUG, "Down key pressed");
      xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);

      if (myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.count > 0) {
        myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.activeView++;

        if (myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.activeView <= myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.count) {
          debug->println(DEBUG_LEVEL_INFO, "Changing to next secondary view");
          changeGauge = false;
        } else {
          debug->println(DEBUG_LEVEL_INFO, "Changing to next view");
          if (myGauges[myDisplays[activeDisplay]->activeView]->getType() == TYPE_DUAL_TEXT) {
            debug->println(DEBUG_LEVEL_DEBUG, "view is dual text");
            myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.activeView = 1;
          } else {
            debug->println(DEBUG_LEVEL_DEBUG, "view is NOT dual text");
            myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.activeView = 0;
          }
        }
      }

      if (changeGauge) {
        myDisplays[activeDisplay]->nextView = myDisplays[activeDisplay]->activeView + 1;
        if (myDisplays[activeDisplay]->nextView > MAX_VIEWS) {
          myDisplays[activeDisplay]->nextView = 1;
        }
        /*if (myDisplays[activeDisplay]->nextView == VIEW_DATE_TIME) {
          memset(oldDateString, 0, DATE_LENGTH);
          memset(oldTimeString, 0, TIME_LENGTH);
        }*/
      }
      mySettings.save();
      xSemaphoreGive(keyPadSemaphore);
      yield();

      vTaskDelay(DELAY_VIEW_CHANGE / portTICK_PERIOD_MS);
    }

    vTaskDelay(DELAY_KEYPAD / portTICK_PERIOD_MS);

    if (testDownKey) {
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }    
  }
}

#endif