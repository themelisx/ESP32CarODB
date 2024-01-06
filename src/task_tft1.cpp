#include <Arduino.h>

#include "defines.h"

#ifdef USE_MULTI_THREAD
#include "debug.h"
#include "gauge.h"
#include "vars.h"
#include "display.h"

void tft1_task(void *pvParameters) {
  debug->print(DEBUG_LEVEL_INFO, "View manager TFT 1: Task running on core ");
  debug->println(DEBUG_LEVEL_INFO, xPortGetCoreID());

  Display *display = displayManager->getDisplay(1);
  
  for (;;) {

    display->updateDisplay();

  }
}

#endif