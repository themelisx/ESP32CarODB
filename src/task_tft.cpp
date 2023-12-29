#include <Arduino.h>

#include "defines.h"

#ifdef USE_MULTI_THREAD
#include "debug.h"
#include "gauge.h"
#include "vars.h"
#include "displays.h"

bool drawActiveGauge(int activeDisplay, int viewIndex) {
  
  bool changeView = false;

  xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
  if (myDisplays[activeDisplay]->activeView != myDisplays[activeDisplay]->nextView || 
    myDisplays[activeDisplay]->secondaryActiveView != myGauges[viewIndex]->secondaryViews.activeView) {

    if (myDisplays[activeDisplay]->nextView == -1) { // First run
      myDisplays[activeDisplay]->nextView = myDisplays[activeDisplay]->activeView;
    }
    debug->print(DEBUG_LEVEL_INFO, "Changing Gauge at display ");
    debug->println(DEBUG_LEVEL_INFO, activeDisplay);

    myDisplays[activeDisplay]->activeView = myDisplays[activeDisplay]->nextView;
    myDisplays[activeDisplay]->secondaryActiveView = myGauges[viewIndex]->secondaryViews.activeView;
    viewIndex = myDisplays[activeDisplay]->nextView;

    debug->print(DEBUG_LEVEL_INFO, "Active gauge: ");
    debug->println(DEBUG_LEVEL_INFO, myGauges[viewIndex]->data.title);

    myDisplays[activeDisplay]->getTFT()->fillScreen(BACK_COLOR);

    myGauges[viewIndex]->data.state = STATE_UNKNOWN;
    myGauges[viewIndex]->data.value = myGauges[viewIndex]->data.min;

    if (myGauges[viewIndex]->getType() == TYPE_GAUGE_GRAPH && myGauges[viewIndex]->secondaryViews.activeView == 0) {
      myGauges[viewIndex]->drawBorders();
    }
    changeView = true;
  }
  xSemaphoreGive(semaphoreActiveView);

  return changeView;
}

void tft1_task(void *pvParameters) {
  debug->print(DEBUG_LEVEL_INFO, "View manager task running on core ");
  debug->println(DEBUG_LEVEL_INFO, xPortGetCoreID());

  int activeDisplay;
  int viewIndex;
  
  for (;;) {

    activeDisplay = getActiveDisplay();
    viewIndex = myDisplays[activeDisplay]->getActiveView();

    if (myGauges[viewIndex]->getId() != VIEW_NONE) {   
      
      if (odbAdapter->isDeviceConnected() && odbAdapter->isOBDConnected()) {

        debug->println(DEBUG_LEVEL_DEBUG, "TFT Loop");
        if (drawActiveGauge(activeDisplay, viewIndex)) {

          debug->println(DEBUG_LEVEL_DEBUG, "Change view request");
          
          activeDisplay = getActiveDisplay();
          viewIndex = myDisplays[activeDisplay]->getActiveView();          
          myGauges[viewIndex]->draw(true);

        } else {

          if (myGauges[viewIndex]->valueHasChanged()) {
            debug->println(DEBUG_LEVEL_DEBUG, "Value has changed");
            myGauges[viewIndex]->draw(false);
          } else {
            debug->println(DEBUG_LEVEL_DEBUG, "Value is equal");
          }
        }

        vTaskDelay(DELAY_REFRESH_VIEW / portTICK_PERIOD_MS);

      } else {
        myDisplays[activeDisplay]->getTFT()->fillScreen(BACK_COLOR);
        delay(500);
        myDisplays[activeDisplay]->printMsg("NO OBD");
        delay(DELAY_MAIN_TASK);
        #ifndef MOCK_OBD
          odbAdapter->connect(nullptr);
        #endif
      }
      
    } else {
      //fill blank screen or msg ?
    }
  }
}

#endif