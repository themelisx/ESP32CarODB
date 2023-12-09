#include <arduino.h>

#include "defines.h"
#include "vars.h"

#include "debug.h"

void main_menu_task(void *pvParameters) {

  debug.print(DEBUG_LEVEL_DEBUG, F("MainManu task running on core "));
  debug.println(DEBUG_LEVEL_DEBUG, xPortGetCoreID());

  for (;;) {

    vTaskDelay(DELAY_MAIN_MENU_TASK / portTICK_PERIOD_MS);
  }
}

