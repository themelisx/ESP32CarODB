#include <Arduino.h>

#include "defines.h"

#ifdef USE_MULTI_THREAD

#include "debug.h"
#include "gauge.h"
#include "vars.h"
#include "display.h"

void keypad_task(void *pvParameters) {
  debug->print(DEBUG_LEVEL_INFO, "Keypad manager task running on core ");
  debug->println(DEBUG_LEVEL_INFO, xPortGetCoreID());

  #ifdef USE_MOCK_KEYPAD
  bool testDownKey = true;
#else
  bool testDownKey = false;
#endif
  unsigned long lastTime = millis();

  for (;;) {

    #ifdef ENABLE_SECOND_DISPLAY
    if (digitalRead(PIN_LEFT_KEY) == BUTTON_PRESSED) { // LEFT KEY PRESSED

      debug->println(DEBUG_LEVEL_DEBUG, "Left key pressed");
      displayManager->goToPreviousDisplay();
      vTaskDelay(DELAY_VIEW_CHANGE / portTICK_PERIOD_MS);

    } else if (digitalRead(PIN_RIGHT_KEY) == BUTTON_PRESSED) {

      debug->println(DEBUG_LEVEL_DEBUG, "Right key pressed");
      displayManager->goToNextDisplay();
      vTaskDelay(DELAY_VIEW_CHANGE / portTICK_PERIOD_MS);

    } else if (digitalRead(PIN_ENTER_KEY) == BUTTON_PRESSED) { // ENTER KEY PRESSED

      debug->println(DEBUG_LEVEL_DEBUG, "Enter key pressed");
      vTaskDelay(DELAY_VIEW_CHANGE / portTICK_PERIOD_MS);
      
    } else 
    #endif
    if (digitalRead(PIN_UP_KEY) == BUTTON_PRESSED) { // UP KEY PRESSED

      debug->println(DEBUG_LEVEL_DEBUG, "Up key pressed");
      displayManager->goToPreviousView();
      vTaskDelay(DELAY_VIEW_CHANGE / portTICK_PERIOD_MS);

    } else if (digitalRead(PIN_DOWN_KEY) == BUTTON_PRESSED || (testDownKey && (millis() - lastTime) > TEST_KEY_DELAY)) { // DOWN KEY PRESSED

      lastTime = millis();
      debug->println(DEBUG_LEVEL_DEBUG, "Down key pressed");
      displayManager->goToNextView();
      vTaskDelay(DELAY_VIEW_CHANGE / portTICK_PERIOD_MS);
    } else {
      vTaskDelay(DELAY_KEYPAD / portTICK_PERIOD_MS);
    }
  }
}

#endif