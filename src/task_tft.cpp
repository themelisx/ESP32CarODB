#include <Arduino.h>

#include "defines.h"

#ifdef USE_MULTI_THREAD
#include "debug.h"
#include "gauge.h"
#include "vars.h"
#include "displays.h"

bool drawActiveGauge(int activeDisplay, int viewId) {
  
  bool changeView = false;

  if (myDisplays[activeDisplay]->activeView != myDisplays[activeDisplay]->nextView || 
    myDisplays[activeDisplay]->secondaryActiveView != myGauges[viewId]->secondaryViews.activeView) {

    if (myDisplays[activeDisplay]->nextView == -1) { // First run
      myDisplays[activeDisplay]->nextView = myDisplays[activeDisplay]->activeView;
    }
    debug->print(DEBUG_LEVEL_INFO, "Changing Gauge at display ");
    debug->println(DEBUG_LEVEL_INFO, activeDisplay);

    myDisplays[activeDisplay]->activeView = myDisplays[activeDisplay]->nextView;
    myDisplays[activeDisplay]->secondaryActiveView = myGauges[viewId]->secondaryViews.activeView;
    viewId = myDisplays[activeDisplay]->nextView;

    debug->print(DEBUG_LEVEL_INFO, "Active gauge: ");
    debug->println(DEBUG_LEVEL_INFO, myGauges[viewId]->data.title);

    myDisplays[activeDisplay]->getTFT()->fillScreen(BACK_COLOR);

    myGauges[viewId]->data.state = STATE_UNKNOWN;
    myGauges[viewId]->data.value = myGauges[viewId]->data.min;

    if (myGauges[viewId]->getType() == TYPE_GAUGE_GRAPH && myGauges[viewId]->secondaryViews.activeView == 0) {
      myGauges[viewId]->drawBorders();
    }
    changeView = true;
  } 

  return changeView;
}

int getActiveView(int activeDisplay) {
  
  int ret;
  xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
  ret = myDisplays[activeDisplay]->activeView;
  xSemaphoreGive(semaphoreActiveView);
  return ret;

}

int getActiveDisplay() {

  int ret;
  xSemaphoreTake(semaphoreActiveDisplay, portMAX_DELAY);
  ret = realActiveDisplay;
  xSemaphoreGive(semaphoreActiveDisplay);
  return ret;

}

void tft1_task(void *pvParameters) {
  debug->print(DEBUG_LEVEL_INFO, "View manager task running on core ");
  debug->println(DEBUG_LEVEL_INFO, xPortGetCoreID());

  int activeDisplay;
  int viewId;
  int viewIndex;
  
  for (;;) {


    activeDisplay = getActiveDisplay();
    viewIndex = getActiveView(activeDisplay);
    
    viewId = myGauges[viewIndex]->getId();

    if (viewId != VIEW_NONE) {   
      
      if (odbAdapter->isDeviceConnected() && odbAdapter->isOBDConnected()) {

        bool changeView = drawActiveGauge(activeDisplay, viewId);
        if (changeView) {
          debug->println(DEBUG_LEVEL_DEBUG, "Change view request");
          activeDisplay = getActiveDisplay();
          viewIndex = getActiveView(activeDisplay);
          viewId = myGauges[viewIndex]->getId();
        }

        bool valueReaded = odbAdapter->readValueForViewType(viewId);

        int newValue = INT_MIN;
        if (valueReaded) {
          
          newValue = odbAdapter->getValueForViewType(viewId);

          //debug->print(DEBUG_LEVEL_DEBUG2, "---> new value : ");
          //debug->println(DEBUG_LEVEL_DEBUG2, newValue);
          //debug->print(DEBUG_LEVEL_DEBUG2, "---> old value : ");
          //debug->println(DEBUG_LEVEL_DEBUG2, myGauges[viewIndex]->data.value);

          bool redrawView = myGauges[viewIndex]->data.value != newValue;
          myGauges[viewIndex]->data.value = newValue;

          // TODO: semaphore for secondraryActiveView
          if (myDisplays[activeDisplay]->secondaryActiveView != VIEW_NONE) {
            int secondaryViewIdx = myGauges[viewIndex]->secondaryViews.activeView;
            int secondaryViewId = myGauges[viewIndex]->secondaryViews.ids[secondaryViewIdx];

            valueReaded = odbAdapter->readValueForViewType(secondaryViewId);
            if (valueReaded) {
              newValue = odbAdapter->getValueForViewType(secondaryViewId);
              if (myGauges[viewIndex]->secondaryViews.oldValue[secondaryViewIdx] != newValue) {
                redrawView = true;
              }
              myGauges[viewIndex]->secondaryViews.value[secondaryViewIdx] = newValue;
            }
          }

          if (redrawView || changeView) { 
            //debug->println(DEBUG_LEVEL_DEBUG, "Draw gauge request");
            myGauges[viewIndex]->drawGauge(changeView);
          }

        } else {
            debug->println(DEBUG_LEVEL_DEBUG2, "value NOT readed");
        }

        vTaskDelay(myGauges[viewId]->getInterval() / portTICK_PERIOD_MS);

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