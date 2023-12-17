#include <Arduino.h>

#include "defines.h"

#ifdef USE_MULTI_THREAD
#include "debug.h"
#include "gauge.h"
#include "vars.h"
#include "displays.h"


void tft1_task(void *pvParameters) {
  debug->print(DEBUG_LEVEL_INFO, "View manager task running on core ");
  debug->println(DEBUG_LEVEL_INFO, xPortGetCoreID());

  int i = 1;
  int viewId = myDisplays[i]->nextView;

  int newValue = INT_MIN;
  bool changeView = true;
  
  for (;;) {

    xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);

    if (myDisplays[i]->activeView != myDisplays[i]->nextView || 
        myDisplays[i]->secondaryActiveView != myGauges[viewId]->secondaryViews.activeView) {

      debug->print(DEBUG_LEVEL_INFO, "Changing Gauge at display ");
      debug->println(DEBUG_LEVEL_INFO, i);

      myDisplays[i]->activeView = myDisplays[i]->nextView;
      myDisplays[i]->secondaryActiveView = myGauges[viewId]->secondaryViews.activeView;
      viewId = myDisplays[i]->nextView;

      debug->print(DEBUG_LEVEL_INFO, "Active gauge: ");
      debug->println(DEBUG_LEVEL_INFO, myGauges[viewId]->data.title);

      changeView = true;

      myDisplays[i]->getTFT()->fillScreen(BACK_COLOR);

      myGauges[viewId]->data.state = STATE_UNKNOWN;
      myGauges[viewId]->data.value = myGauges[viewId]->data.min;

      if (myGauges[viewId]->getType() == TYPE_GAUGE_GRAPH && myGauges[viewId]->secondaryViews.activeView == 0) {
        myGauges[viewId]->drawBorders();
      }
    } else {
      changeView = false;
    }
    xSemaphoreGive(keyPadSemaphore);
    yield();

    if (odbAdapter.isDeviceConnected() && odbAdapter.isOBDConnected()) {
      xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
      switch (viewId) {
        case VIEW_BATTERY_VOLTAGE: newValue = odbAdapter.getVoltage(); break;
        case VIEW_KPH: newValue = odbAdapter.getKph(); break;
        case VIEW_RPM: newValue = odbAdapter.getRpm(); break;
        case VIEW_COOLANT_TEMP: newValue = odbAdapter.getCoolantTemp(); break;
        case VIEW_AMBIENT_TEMP: newValue = odbAdapter.getAmbientTemp(); break;
        case VIEW_INTAKE_TEMP: newValue = odbAdapter.getIntakeTemp(); break;
        case VIEW_TIMING_ADV: newValue = odbAdapter.getTimingAdvance(); break;
        default: newValue = 0;
      }
      xSemaphoreGive(obdValueSemaphore);
      yield();
    }
    
    xSemaphoreTake(myGauges[viewId]->semaphore, portMAX_DELAY);
    int oldValue = myGauges[viewId]->data.value;
    xSemaphoreGive(myGauges[viewId]->semaphore);
    yield();

    /*if (viewId == VIEW_DATE_TIME) {
      myGauges[viewId]->drawDateTime();
    } else*/ 
    if (changeView || oldValue != newValue) {
      //debug->print(DEBUG_LEVEL_INFO, "Readed value: ");
      //debug->println(DEBUG_LEVEL_INFO, newValue);
      //debug->println(DEBUG_LEVEL_INFO, "Drawing....");
      myGauges[viewId]->drawGauge(viewId, changeView, newValue);
    }

    vTaskDelay(myGauges[viewId]->getInterval() / portTICK_PERIOD_MS);
  }
}

#endif