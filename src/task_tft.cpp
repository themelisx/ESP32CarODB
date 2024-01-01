#include <Arduino.h>

#include "defines.h"

#ifdef USE_MULTI_THREAD
#include "debug.h"
#include "gauge.h"
#include "vars.h"
#include "display.h"

void tft1_task(void *pvParameters) {
  debug->print(DEBUG_LEVEL_INFO, "View manager task running on core ");
  debug->println(DEBUG_LEVEL_INFO, xPortGetCoreID());

  Display *display;
  Gauge *gauge;
  
  for (;;) {

    display = displayManager->getDisplay(displayManager->getActiveDisplayId());
    gauge = display->getActiveGauge();

    if (odbAdapter->isDeviceConnected() && odbAdapter->isOBDConnected()) {

      debug->println(DEBUG_LEVEL_DEBUG, "TFT Loop");

      bool changeView = false;

      xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
      if (display->activeView != display->nextView || 
        display->secondaryActiveView != gauge->secondaryViews.activeView) {

        if (display->nextView == -1) { // First run
          display->nextView = display->activeView;
        }
        debug->print(DEBUG_LEVEL_INFO, "Changing Gauge at display ");
        debug->println(DEBUG_LEVEL_INFO, display->getId());

        display->activeView = display->nextView;
        display->secondaryActiveView = gauge->secondaryViews.activeView;

        debug->print(DEBUG_LEVEL_INFO, "Active gauge: ");
        debug->println(DEBUG_LEVEL_INFO, gauge->data.title);

        display->getTFT()->fillScreen(BACK_COLOR);

        gauge->data.state = STATE_UNKNOWN;
        gauge->data.value = gauge->data.min;

        if (gauge->getType() == TYPE_GAUGE_GRAPH && gauge->secondaryViews.activeView == 0) {
          gauge->drawBorders();
        }
        changeView = true;
      }
      xSemaphoreGive(semaphoreActiveView);

      if (changeView) {

        debug->println(DEBUG_LEVEL_DEBUG, "Change view request");
        
        //activeDisplay = getActiveDisplay();
        //viewIndex = display->getActiveView();          
        gauge->draw(true);

      } else {

        if (gauge->valueHasChanged()) {
          debug->println(DEBUG_LEVEL_DEBUG, "Value has changed");
          gauge->draw(false);
        } else {
          debug->println(DEBUG_LEVEL_DEBUG, "Value is equal");
        }
      }

      vTaskDelay(DELAY_REFRESH_VIEW / portTICK_PERIOD_MS);

    } else {
      display->getTFT()->fillScreen(BACK_COLOR);
      delay(500);
      display->printMsg("NO OBD");
      delay(DELAY_MAIN_TASK);
      #ifndef MOCK_OBD
        odbAdapter->connect(nullptr);
      #endif
    }

  }
}

#endif