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
  unsigned long lastTime = millis();

  for (;;) {

    if (digitalRead(PIN_LEFT_KEY) == LOW) { // LEFT KEY PRESSED

      debug->println(DEBUG_LEVEL_DEBUG, "Left key pressed");
      
      xSemaphoreTake(semaphoreActiveDisplay, portMAX_DELAY);
      realActiveDisplay--;
      if (realActiveDisplay < 1) {
        realActiveDisplay = MAX_DISPLAYS;
      }
      xSemaphoreGive(semaphoreActiveDisplay);
      yield();

    } else if (digitalRead(PIN_RIGHT_KEY) == LOW) {

      debug->println(DEBUG_LEVEL_DEBUG, "Right key pressed");
      
      xSemaphoreTake(semaphoreActiveDisplay, portMAX_DELAY);
      realActiveDisplay++;
      if (realActiveDisplay > MAX_DISPLAYS) {
        realActiveDisplay = 1;
      }
      xSemaphoreGive(semaphoreActiveDisplay);
      yield();

    } else if (digitalRead(PIN_ENTER_KEY) == LOW) { // ENTER KEY PRESSED

      debug->println(DEBUG_LEVEL_DEBUG, "Enter key pressed");
      
    } else if (digitalRead(PIN_UP_KEY) == LOW) { // UP KEY PRESSED

      debug->println(DEBUG_LEVEL_DEBUG, "Up key pressed");

      bool changeGauge = true;
      xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);

      if (myGauges[myDisplays[realActiveDisplay]->activeView]->secondaryViews.count > 0) {
        myGauges[myDisplays[realActiveDisplay]->activeView]->secondaryViews.activeView--;

        if (myGauges[myDisplays[realActiveDisplay]->activeView]->secondaryViews.activeView > 0) {
          debug->print(DEBUG_LEVEL_INFO, "Changing to prev secondary view");
          changeGauge = false;
        } else {
          myGauges[myDisplays[realActiveDisplay]->activeView]->secondaryViews.activeView = myGauges[myDisplays[realActiveDisplay]->activeView]->secondaryViews.count;
        }
      }

      if (changeGauge) {
        myDisplays[realActiveDisplay]->nextView = myDisplays[realActiveDisplay]->activeView - 1;
        if (myDisplays[realActiveDisplay]->nextView == 0) {
          myDisplays[realActiveDisplay]->nextView = MAX_VIEWS;
        }
        /*if (myDisplays[realActiveDisplay]->nextView == VIEW_DATE_TIME) {
          memset(oldDateString, 0, DATE_LENGTH);
          memset(oldTimeString, 0, TIME_LENGTH);
        }*/
      }
      xSemaphoreGive(semaphoreActiveView);

      mySettings->setActiveView(myDisplays[1]->activeView);
      mySettings->setSecondaryActiveView(myDisplays[1]->secondaryActiveView);
      mySettings->save();      
      yield();

    } else if (digitalRead(PIN_DOWN_KEY) == LOW || (testDownKey && (millis() - lastTime) > TEST_KEY_DELAY)) { // DOWN KEY PRESSED

      lastTime = millis();
      bool changeGauge = true;

      debug->println(DEBUG_LEVEL_DEBUG, "Down key pressed");
      xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);

      if (myGauges[myDisplays[realActiveDisplay]->activeView]->secondaryViews.count > 0) {
        myGauges[myDisplays[realActiveDisplay]->activeView]->secondaryViews.activeView++;

        if (myGauges[myDisplays[realActiveDisplay]->activeView]->secondaryViews.activeView <= myGauges[myDisplays[realActiveDisplay]->activeView]->secondaryViews.count) {
          debug->println(DEBUG_LEVEL_INFO, "Changing to next secondary view");
          changeGauge = false;
        } else {
          debug->println(DEBUG_LEVEL_INFO, "Changing to next view");
          if (myGauges[myDisplays[realActiveDisplay]->activeView]->getType() == TYPE_DUAL_TEXT) {
            debug->println(DEBUG_LEVEL_DEBUG, "view is dual text");
            myGauges[myDisplays[realActiveDisplay]->activeView]->secondaryViews.activeView = 1;
          } else {
            debug->println(DEBUG_LEVEL_DEBUG, "view is NOT dual text");
            myGauges[myDisplays[realActiveDisplay]->activeView]->secondaryViews.activeView = 0;
          }
        }
      }

      if (changeGauge) {
        myDisplays[realActiveDisplay]->nextView = myDisplays[realActiveDisplay]->activeView + 1;
        if (myDisplays[realActiveDisplay]->nextView > MAX_VIEWS) {
          myDisplays[realActiveDisplay]->nextView = 1;
        }
        /*if (myDisplays[realActiveDisplay]->nextView == VIEW_DATE_TIME) {
          memset(oldDateString, 0, DATE_LENGTH);
          memset(oldTimeString, 0, TIME_LENGTH);
        }*/
      }
      
      xSemaphoreGive(semaphoreActiveView);

      mySettings->setActiveView(myDisplays[1]->activeView);
      mySettings->setSecondaryActiveView(myDisplays[1]->secondaryActiveView);
      mySettings->save();
      
      yield();
      vTaskDelay(DELAY_VIEW_CHANGE / portTICK_PERIOD_MS);
    }

    vTaskDelay(DELAY_KEYPAD / portTICK_PERIOD_MS);
  }
}

#endif