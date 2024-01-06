#include <Arduino.h>
#ifdef ESP32
    #include <WiFi.h>
  #endif
  #ifdef ESP8266
    #include <ESP8266WiFi.h>
  #endif

#include <ELMduino.h>

#include "defines.h"
#include "debug.h"
#include "gauge.h"
#include "displayManager.h"
#include "structs.h"
#include "settings.h"

#include "odbAdapter.h"

#ifdef USE_MULTI_THREAD
  #include "tasks.h"
#endif

#ifdef ENABLE_RTC_CLOCK
  #include "myRTC.h"
#endif

#ifdef USE_MULTI_THREAD
  // Tasks
  TaskHandle_t t_core0_tft1;
  TaskHandle_t t_core0_tft2;
  TaskHandle_t t_core0_keypad;
  TaskHandle_t t_core1_obd;

  // Semaphores
  SemaphoreHandle_t semaphoreActiveDisplay;
  SemaphoreHandle_t semaphoreActiveView;
  SemaphoreHandle_t semaphoreData;
  SemaphoreHandle_t btConnectedSemaphore;
  SemaphoreHandle_t obdConnectedSemaphore;
#else
  Gauge* gauge;  

  unsigned long lastTime;
  //int viewId;
  //int viewIndex;
#endif

Display* display;

Debug *debug;
ELM327 *obd;
OdbAdapter *odbAdapter;
DisplayManager *displayManager;

#ifdef ENABLE_RTC_CLOCK
  RTC_DS1302 myRTC;

  S_DateTime dateTime;

  char 
  String[DATE_LENGTH];
  char timeString[TIME_LENGTH];
  char oldDateString[DATE_LENGTH];
  char oldTimeString[TIME_LENGTH];
#endif

Settings *mySettings;

#ifdef USE_MOCK_KEYPAD
  bool testDownKey = true;
#else
  bool testDownKey = false;
#endif

void setup() {

  // Initialize Serial and set debug level
  debug = new Debug();
  debug->start(115200, DEBUG_LEVEL_INFO);

  debug->println(DEBUG_LEVEL_INFO, "Staring up...");

#ifdef USE_MULTI_THREAD
  btConnectedSemaphore = xSemaphoreCreateMutex();
  obdConnectedSemaphore = xSemaphoreCreateMutex();
  semaphoreActiveDisplay = xSemaphoreCreateMutex();
  semaphoreActiveView = xSemaphoreCreateMutex();
  semaphoreData = xSemaphoreCreateMutex();

  xSemaphoreGive(btConnectedSemaphore);
  xSemaphoreGive(obdConnectedSemaphore);
  xSemaphoreGive(semaphoreActiveDisplay);
  xSemaphoreGive(semaphoreActiveView);
  xSemaphoreGive(semaphoreData);
#endif

  displayManager = new DisplayManager();
  displayManager->addDisplay(1, TFT1_CS, TFT1_DC, TFT1_WIDTH, TFT1_HEIGHT);
  #ifdef ENABLE_STARTUP_LOGO
    displayManager->showStartupLogo(1);
  #endif

  #ifdef ENABLE_SECOND_DISPLAY
    displayManager->addDisplay(2, TFT2_CS, TFT2_DC, TFT1_WIDTH, TFT1_HEIGHT);
    #ifdef ENABLE_STARTUP_LOGO
      displayManager->showStartupLogo(2);
    #endif
  #endif
  
  mySettings = new Settings();
  mySettings->load();

  #ifdef USE_OBD_BLUETOOTH
    //odbAdapter = new OdbAdapter("OBDII", "11:22:33:dd:ee:ff");
    odbAdapter = new OdbAdapter("OBDII", "00:12:6f:10:24:aa");

    debug->println(DEBUG_LEVEL_INFO, "Disabling WiFi...");  
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  #endif
  #ifdef USE_OBD_WIFI
    odbAdapter = new OdbAdapter("OBDII", "192.168.0.10");
  #endif

  #ifdef ENABLE_RTC_CLOCK
    myRTC.start();
  #endif

  pinMode(PIN_UP_KEY, INPUT);
  pinMode(PIN_DOWN_KEY, INPUT);
  #ifdef ENABLE_SECOND_DISPLAY
    pinMode(PIN_LEFT_KEY, INPUT);
    pinMode(PIN_RIGHT_KEY, INPUT);
    pinMode(PIN_ENTER_KEY, INPUT);
  #endif

  debug->println(DEBUG_LEVEL_INFO, "Creating gauges...");
  display = displayManager->getDisplay(1);
  displayManager->setActiveDisplay(1);
  
  // Battery  
  display->addGauge(VIEW_BATTERY_VOLTAGE, TYPE_GAUGE_GRAPH, DELAY_VIEW_BATTERY_VOLTAGE, (char*)"Volt", (char*)"%0.1f", RED, RED, true, true, 110, 120, 140, 150);
  // KM/h
  display->addGauge(VIEW_KPH, TYPE_GAUGE_GRAPH, DELAY_VIEW_KPH, (char*)"Km/h", (char*)"%d", WHITE, RED, false, false, 0, 0, 130, 200);
  display->addSecondaryView(VIEW_KPH, VIEW_RPM, (char*)"%d");
  // RPM  
  display->addGauge(VIEW_RPM, TYPE_GAUGE_GRAPH, DELAY_VIEW_RPM, (char*)"RPM", (char*)"%d", WHITE, RED, false, true, 0, 0, 6000, 7500);
  display->addSecondaryView(VIEW_RPM, VIEW_KPH, (char*)"%d");
  // Engine coolant  
  display->addGauge(VIEW_COOLANT_TEMP, TYPE_GAUGE_GRAPH, DELAY_VIEW_COOLANT_TEMP, (char*)"Engine", (char*)"%d C", BLUE, RED, true, true, 0, 40, 105, 120);
  display->addSecondaryView(VIEW_COOLANT_TEMP, VIEW_INTAKE_TEMP, (char*)"%d C");
  // Intake
  display->addGauge(VIEW_INTAKE_TEMP, TYPE_GAUGE_GRAPH, DELAY_VIEW_INTAKE_AIR_TEMP, (char*)"Intake", (char*)"%d C", WHITE, RED, false, true, -20, -20, 65, 100);
  display->addSecondaryView(VIEW_INTAKE_TEMP, VIEW_COOLANT_TEMP, (char*)"%d C");
  // Advance
  display->addGauge(VIEW_TIMING_ADV, TYPE_SIMPLE_TEXT, DELAY_VIEW_TIMING_ADV, (char*)"Advance", (char*)"%d º", WHITE, WHITE, false, false, 0, 0, 50, 50);
  // Throttle
  display->addGauge(VIEW_THROTTLE, TYPE_GAUGE_GRAPH, DELAY_VIEW_THROTTLE, (char*)"THROTL", (char*)"%d", WHITE, WHITE, false, false, 0, 0, 100, 100);
  display->addSecondaryView(VIEW_THROTTLE, VIEW_ENGINE_LOAD, (char*)"%d");
  //  Engine load
  display->addGauge(VIEW_ENGINE_LOAD, TYPE_GAUGE_GRAPH, DELAY_VIEW_ENGINE_LOAD, (char*)"Load", (char*)"%d", WHITE, WHITE, false, false, 0, 0, 100, 100);
  display->addSecondaryView(VIEW_ENGINE_LOAD, VIEW_THROTTLE, (char*)"%d");
  // Short fuel trims
  //display->addGauge(VIEW_SHORT_FUEL_TRIM, TYPE_DUAL_TEXT, DELAY_VIEW_SHORT_FUEL_TRIM, (char*)"S.F.T.", (char*)"%d", RED, RED, false, false, -30, -20, 20, 30);
  //display->addSecondaryView(VIEW_SHORT_FUEL_TRIM, VIEW_LONG_FUEL_TRIM, (char*)"%d");
  // Long fuel trims
  //display->addGauge(VIEW_LONG_FUEL_TRIM, TYPE_DUAL_TEXT, DELAY_VIEW_LONG_FUEL_TRIM, (char*)"L.F.T.", (char*)"%d", RED, RED, false, false, -30, -20, 20, 30);
  //display->addSecondaryView(VIEW_LONG_FUEL_TRIM, VIEW_SHORT_FUEL_TRIM, (char*)"%d");
  // MAF rate
  //display->addGauge(VIEW_MAF_RATE, TYPE_GAUGE_GRAPH, DELAY_VIEW_MAF_RATE, (char*)"MAF", (char*)"%d", RED, RED, false, false, -10, 0, 10, 10);
  // Fuel level
  //display->addGauge(VIEW_FUEL_LEVEL, TYPE_GAUGE_GRAPH, DELAY_VIEW_FUEL_LEVEL, (char*)"FUEL", (char*)"%d", RED, WHITE, true, false, 0, 15, 100, 100);
  // Ambient  
  //display->addGauge(VIEW_AMBIENT_TEMP, TYPE_GAUGE_GRAPH, DELAY_VIEW_AMBIENT_AIR_TEMP, (char*)"Out", (char*)"%d C", BLUE, WHITE, true, false, -30, 3, 50, 50);
  // Oil temp
  //display->addGauge(VIEW_OIL_TEMP, TYPE_GAUGE_GRAPH, DELAY_VIEW_OIL_TEMP, (char*)"OIL", (char*)"%d C", BLUE, RED, true, true, -30, 40, 110, 150);
  // Abs Load
  //display->addGauge(VIEW_ABS_LOAD, TYPE_GAUGE_GRAPH, DELAY_VIEW_ABS_LOAD, (char*)"A LOAD", (char*)"%d", WHITE, RED, false, false, 0, 0, 100, 100);
 
  //display->addGauge(VIEW_DATE_TIME, TYPE_DATE, DELAY_VIEW_DATE_TIME, (char*)"  ", (char*)"  ", 0, 0, false, false, 0, 0, 0, 0);

  display->setActiveView(mySettings->getActiveView(1));
  display->setNextView(-1);
  display->setSecondaryActiveView(mySettings->getSecondaryActiveView(1));

  #ifdef ENABLE_SECOND_DISPLAY
    Display *display2 = displayManager->getDisplay(2);
    display2->setActiveView(mySettings->getActiveView(2));
    display2->setNextView(-1);
    display2->setSecondaryActiveView(mySettings->getSecondaryActiveView(2));
  #endif

#ifdef USE_MULTI_THREAD
  
  debug->println(DEBUG_LEVEL_INFO, "Staring up View Manager 1...");

  xTaskCreatePinnedToCore(
    tft1_task,        // Task function.
    "View_Manager1",  // Name of task.
    10000,            // Stack size of task
    NULL,             // Parameter of the task
    0,                // Priority of the task
    &t_core0_tft1,    // Task handle to keep track of created task
    0);               // Pin task to core 0
  vTaskSuspend(t_core0_tft1);

  debug->println(DEBUG_LEVEL_INFO, "Staring up OBD Manager...");

  xTaskCreatePinnedToCore(
    obd_task,       // Task function.
    "OBD_Manager",  // Name of task.
    10000,          // Stack size of task
    NULL,           // Parameter of the task
    0,              // Priority of the task
    &t_core1_obd,   // Task handle to keep track of created task
    1);             // Pin task to core 1

  vTaskSuspend(t_core1_obd);

  debug->println(DEBUG_LEVEL_INFO, "Staring up Keypad Manager...");

  xTaskCreatePinnedToCore(
    keypad_task,      // Task function.
    "Keypad_Manager", // Name of task.
    10000,            // Stack size of task
    NULL,             // Parameter of the task
    0,                // Priority of the task
    &t_core0_keypad,  // Task handle to keep track of created task
    1);               // Pin task to core 1

  vTaskSuspend(t_core0_keypad);

  debug->println(DEBUG_LEVEL_INFO, "Setup completed\nStarting tasks...");

  vTaskResume(t_core0_tft1);
  delay(200);
  vTaskResume(t_core0_keypad);
  delay(200);
  
  debug->println(DEBUG_LEVEL_INFO, "Starting OBD...");
  vTaskResume(t_core1_obd);

#else

  #ifndef MOCK_OBD
    // if device does not have pin use the follow
    //odbAdapter->connect(nullptr);
    //bool connected = odbAdapter->connect(OBD_DEVICE_PIN);
  #else
    odbAdapter->setDeviceConnected(true);
    odbAdapter->setObdConnected(true);
  #endif

  lastTime = millis();
#endif

  debug->println(DEBUG_LEVEL_INFO, "Setup completed");

}

#ifndef USE_MULTI_THREAD
void checkKeypad() {
  // KeyPad
  #ifdef ENABLE_SECOND_DISPLAY
  if (digitalRead(PIN_LEFT_KEY) == HIGH) { // LEFT KEY PRESSED

    debug->println(DEBUG_LEVEL_DEBUG, "Left key pressed");
    displayManager->goToPreviousDisplay();
    delay(DELAY_VIEW_CHANGE);

  } else if (digitalRead(PIN_RIGHT_KEY) == HIGH) {

    debug->println(DEBUG_LEVEL_DEBUG, "Right key pressed");
    displayManager->goToNextDisplay();
    delay(DELAY_VIEW_CHANGE);

  } else if (digitalRead(PIN_ENTER_KEY) == HIGH) { // ENTER KEY PRESSED

    debug->println(DEBUG_LEVEL_DEBUG, "Enter key pressed");
    delay(DELAY_VIEW_CHANGE);
    
  } else 
  #endif
  if (digitalRead(PIN_UP_KEY) == LOW) { // UP KEY PRESSED

    debug->println(DEBUG_LEVEL_DEBUG, "Up key pressed");
    displayManager->goToPreviousView();
    delay(DELAY_VIEW_CHANGE);

  } else if (digitalRead(PIN_DOWN_KEY) == LOW || (testDownKey && (millis() - lastTime) > TEST_KEY_DELAY)) { // DOWN KEY PRESSED

    lastTime = millis();
    debug->println(DEBUG_LEVEL_DEBUG, "Down key pressed");
    displayManager->goToNextView();
    delay(DELAY_VIEW_CHANGE);
  } else {
    delay(DELAY_KEYPAD);
  }
}

#endif




void loop() {
  
#ifdef USE_MULTI_THREAD
  //vTaskDelete(NULL);
  vTaskDelay(DELAY_MAIN_TASK / portTICK_PERIOD_MS);
#else

  debug->println(DEBUG_LEVEL_DEBUG2, "--- LOOP ---");

  gauge = display->getActiveGauge();

  if (odbAdapter->isDeviceConnected() && odbAdapter->isOBDConnected()) {

    if (display->getActiveViewIndex() != display->getNextView() || 
      display->getSecondaryActiveView() != gauge->secondaryViews.activeViewIndex) {

      if (display->getNextView() == -1) { // First run
        display->setNextView(display->getActiveViewIndex());
      }
      debug->print(DEBUG_LEVEL_INFO, "Changing Gauge at display ");
      debug->println(DEBUG_LEVEL_INFO, display->getActiveViewId());

      display->setActiveView(display->getNextView());
      display->setSecondaryActiveView(gauge->secondaryViews.activeViewIndex);

      gauge = display->getActiveGauge();

      debug->print(DEBUG_LEVEL_INFO, "Active gauge: ");
      debug->println(DEBUG_LEVEL_INFO, gauge->data.title);

      display->fillScreen(BACK_COLOR);

      gauge->data.state = STATE_UNKNOWN;
      gauge->data.value = gauge->data.min;

      if (gauge->getType() == TYPE_GAUGE_GRAPH && gauge->secondaryViews.activeViewIndex == 0) {
        gauge->drawBorders();
      }
      gauge->setRepaint(true);
      debug->println(DEBUG_LEVEL_DEBUG, "Change view request");
    } 

    bool valueReaded = odbAdapter->readValueForViewType(display->getActiveViewId());

    int newValue = INT_MIN;
    if (valueReaded) {
      
      newValue = odbAdapter->getValueForViewType(display->getActiveViewId());

      //debug->print(DEBUG_LEVEL_DEBUG2, "---> new value : ");
      //debug->println(DEBUG_LEVEL_DEBUG2, newValue);
      //debug->print(DEBUG_LEVEL_DEBUG2, "---> old value : ");
      //debug->println(DEBUG_LEVEL_DEBUG2, gauge->data.value);

      bool redrawView = gauge->data.value != newValue;
      gauge->data.value = newValue;

      int secondaryViewIdx = gauge->secondaryViews.activeViewIndex;
      if (secondaryViewIdx != 0) {
        int secondaryViewId = gauge->secondaryViews.ids[secondaryViewIdx];

        valueReaded = odbAdapter->readValueForViewType(secondaryViewId);
        if (valueReaded) {
          newValue = odbAdapter->getValueForViewType(secondaryViewId);
          if (gauge->secondaryViews.oldValue[secondaryViewIdx] != newValue) {
            redrawView = true;
          }
          gauge->secondaryViews.value[secondaryViewIdx] = newValue;
        }
      }

      if (redrawView || gauge->needsRepaint()) { 
        //debug->println(DEBUG_LEVEL_DEBUG, "Draw gauge request");
        gauge->draw();
      }

    } else {
        debug->println(DEBUG_LEVEL_DEBUG2, "value NOT readed");
    }

    checkKeypad();

    delay(gauge->getInterval());

  } else {
    display->fillScreen(BACK_COLOR);
    delay(500);
    display->printMsg("NO OBD");
    delay(DELAY_MAIN_TASK);
    #ifndef MOCK_OBD
      odbAdapter->connect(nullptr);
    #endif
  }

#endif
}


