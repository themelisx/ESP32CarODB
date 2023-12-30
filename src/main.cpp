#include <Arduino.h>
#include <WiFi.h>
#include <ELMduino.h>

#include "defines.h"
#include "debug.h"
#include "gauge.h"
#include "displays.h"
#include "structs.h"
#include "settings.h"

#include "odbAdapter.h"

#ifdef USE_MULTI_THREAD
  #include "tasks.h"
#endif

#ifdef ENABLE_RTC_CLOCK
  #include "myRTC.h"
#endif

// TFT SPI
Adafruit_GC9A01A tft1(TFT1_CS, TFT1_DC);

#ifdef ENABLE_SECOND_DISPLAY
  Adafruit_GC9A01A tft2(TFT2_CS, TFT2_DC);
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

int realActiveDisplay;
#else
unsigned long lastTime;
int viewId;
int viewIndex;
int activeDisplay;
#endif

Debug *debug;
ELM327 *obd;
OdbAdapter *odbAdapter;

#ifdef ENABLE_RTC_CLOCK
  RTC_DS1302 myRTC;

  S_DateTime dateTime;

  char 
  String[DATE_LENGTH];
  char timeString[TIME_LENGTH];
  char oldDateString[DATE_LENGTH];
  char oldTimeString[TIME_LENGTH];
#endif

Displays *myDisplays[MAX_DISPLAYS + 1];
Gauge *myGauges[MAX_VIEWS + 1];
Settings *mySettings;

#ifdef USE_MOCK_KEYPAD
  bool testDownKey = true;
#else
  bool testDownKey = false;
#endif

#ifdef USE_MULTI_THREAD
int getActiveDisplay() {

  int ret;
  xSemaphoreTake(semaphoreActiveDisplay, portMAX_DELAY);
  ret = realActiveDisplay;
  xSemaphoreGive(semaphoreActiveDisplay);
  return ret;

}
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

  mySettings = new Settings();
  mySettings->load();

  #ifdef USE_OBD_BLUETOOTH
    //odbAdapter = new OdbAdapter("OBDII", "11:22:33:dd:ee:ff");
    odbAdapter = new OdbAdapter("OBDII", "00:12:6f:10:24:aa");

    debug->println(DEBUG_LEVEL_INFO, "Disabling WiFi...");  
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  #endif

  debug->println(DEBUG_LEVEL_INFO, "Staring up TFT1...");
  tft1.begin();
  tft1.setRotation(0);
  tft1.fillScreen(BLACK);

  #ifdef ENABLE_SECOND_DISPLAY
    debug->println(DEBUG_LEVEL_INFO, "Staring up TFT2...");
    tft2.begin();
    tft2.setRotation(0);
    tft2.fillScreen(BLACK);
  #endif

#ifdef ENABLE_STARTUP_LOGO
    tft1.drawBitmap(0, 0, epd_logo1, SCREEN_H, SCREEN_W, WHITE);
    yield();
    #ifdef ENABLE_SECOND_DISPLAY
      tft2.drawBitmap(0, 0, epd_logo2, SCREEN_H, SCREEN_W, WHITE);    
      yield();
    #endif
#endif

#ifdef ENABLE_RTC_CLOCK
  myRTC.start();
#endif

  myDisplays[1] = new Displays(&tft1, 240, 240);
  #ifdef ENABLE_SECOND_DISPLAY
    myDisplays[2] = new Displays(&tft2, 240, 240);
  #endif
  
  debug->println(DEBUG_LEVEL_INFO, "Creating gauges...");
  
  // Battery  
  myGauges[1] = new Gauge(myDisplays[1], VIEW_BATTERY_VOLTAGE, TYPE_GAUGE_GRAPH, DELAY_VIEW_BATTERY_VOLTAGE, (char*)"Volt", (char*)"%0.1f", RED, RED, true, true, 110, 120, 140, 150);
  // KM/h
  myGauges[2] = new Gauge(myDisplays[1], VIEW_KPH, TYPE_GAUGE_GRAPH, DELAY_VIEW_KPH, (char*)"Km/h", (char*)"%d", WHITE, RED, false, false, 0, 0, 130, 200);
  myGauges[2]->addSecondaryView(VIEW_RPM, (char*)"%d");
  // RPM  
  myGauges[3] = new Gauge(myDisplays[1], VIEW_RPM, TYPE_GAUGE_GRAPH, DELAY_VIEW_RPM, (char*)"RPM", (char*)"%d", WHITE, RED, false, true, 0, 0, 6000, 7500);
  myGauges[3]->addSecondaryView(VIEW_KPH, (char*)"%d");
  // Engine coolant  
  myGauges[4] = new Gauge(myDisplays[1], VIEW_COOLANT_TEMP, TYPE_GAUGE_GRAPH, DELAY_VIEW_COOLANT_TEMP, (char*)"Engine", (char*)"%d C", BLUE, RED, true, true, 0, 40, 105, 120);
  myGauges[4]->addSecondaryView(VIEW_INTAKE_TEMP, (char*)"%d C");
  // Intake
  myGauges[5] = new Gauge(myDisplays[1], VIEW_INTAKE_TEMP, TYPE_GAUGE_GRAPH, DELAY_VIEW_INTAKE_AIR_TEMP, (char*)"Intake", (char*)"%d C", WHITE, RED, false, true, -20, -20, 65, 100);
  myGauges[5]->addSecondaryView(VIEW_COOLANT_TEMP, (char*)"%d C");
  // Advance
  myGauges[6] = new Gauge(myDisplays[1], VIEW_TIMING_ADV, TYPE_SIMPLE_TEXT, DELAY_VIEW_TIMING_ADV, (char*)"Advance", (char*)"%d º", WHITE, WHITE, false, false, 0, 0, 50, 50);
  // Throttle
  myGauges[7] = new Gauge(myDisplays[1], VIEW_THROTTLE, TYPE_GAUGE_GRAPH, DELAY_VIEW_THROTTLE, (char*)"THROTL", (char*)"%d", WHITE, WHITE, false, false, 0, 0, 100, 100);
  myGauges[7]->addSecondaryView(VIEW_ENGINE_LOAD, (char*)"%d");
  //  Engine load
  myGauges[8] = new Gauge(myDisplays[1], VIEW_ENGINE_LOAD, TYPE_GAUGE_GRAPH, DELAY_VIEW_ENGINE_LOAD, (char*)"Load", (char*)"%d", WHITE, WHITE, false, false, 0, 0, 100, 100);
  myGauges[8]->addSecondaryView(VIEW_THROTTLE, (char*)"%d");
  // Short fuel trims
  myGauges[9] = new Gauge(myDisplays[1], VIEW_SHORT_FUEL_TRIM, TYPE_DUAL_TEXT, DELAY_VIEW_SHORT_FUEL_TRIM, (char*)"S.F.T.", (char*)"%d", RED, RED, false, false, -30, -20, 20, 30);
  myGauges[9]->addSecondaryView(VIEW_LONG_FUEL_TRIM, (char*)"%d");
  // Long fuel trims
  myGauges[10] = new Gauge(myDisplays[1], VIEW_LONG_FUEL_TRIM, TYPE_DUAL_TEXT, DELAY_VIEW_LONG_FUEL_TRIM, (char*)"L.F.T.", (char*)"%d", RED, RED, false, false, -30, -20, 20, 30);
  myGauges[10]->addSecondaryView(VIEW_SHORT_FUEL_TRIM, (char*)"%d");
  // MAF rate
  //myGauges[11] = new Gauge(myDisplays[1], VIEW_MAF_RATE, TYPE_GAUGE_GRAPH, DELAY_VIEW_MAF_RATE, (char*)"MAF", (char*)"%d", RED, RED, false, false, -10, 0, 10, 10);
  // Fuel level
  //myGauges[12] = new Gauge(myDisplays[1], VIEW_FUEL_LEVEL, TYPE_GAUGE_GRAPH, DELAY_VIEW_FUEL_LEVEL, (char*)"FUEL", (char*)"%d", RED, WHITE, true, false, 0, 15, 100, 100);
  // Ambient  
  //myGauges[13] = new Gauge(myDisplays[1], VIEW_AMBIENT_TEMP, TYPE_GAUGE_GRAPH, DELAY_VIEW_AMBIENT_AIR_TEMP, (char*)"Out", (char*)"%d C", BLUE, WHITE, true, false, -30, 3, 50, 50);
  // Oil temp
  //myGauges[14] = new Gauge(myDisplays[1], VIEW_OIL_TEMP, TYPE_GAUGE_GRAPH, DELAY_VIEW_OIL_TEMP, (char*)"OIL", (char*)"%d C", BLUE, RED, true, true, -30, 40, 110, 150);
  // Abs Load
  //myGauges[15] = new Gauge(myDisplays[1], VIEW_ABS_LOAD, TYPE_GAUGE_GRAPH, DELAY_VIEW_ABS_LOAD, (char*)"A LOAD", (char*)"%d", WHITE, RED, false, false, 0, 0, 100, 100);
 
  //myGauges[8] = new Gauge(myDisplays[1], VIEW_DATE_TIME, TYPE_DATE, DELAY_VIEW_DATE_TIME, (char*)"  ", (char*)"  ", 0, 0, false, false, 0, 0, 0, 0);

  pinMode(PIN_UP_KEY, INPUT);
  pinMode(PIN_DOWN_KEY, INPUT);
  #ifdef ENABLE_SECOND_DISPLAY
  pinMode(PIN_LEFT_KEY, INPUT);
  pinMode(PIN_RIGHT_KEY, INPUT);
  pinMode(PIN_ENTER_KEY, INPUT);
  #endif

#ifdef USE_MULTI_THREAD
  realActiveDisplay = 1;
  myDisplays[realActiveDisplay]->activeView = mySettings->getActiveView();
  myDisplays[realActiveDisplay]->nextView = -1;
  myDisplays[realActiveDisplay]->secondaryActiveView = mySettings->getSecondaryActiveView();

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

  activeDisplay = 1;
  myDisplays[activeDisplay]->activeView = mySettings->getActiveView();
  myDisplays[activeDisplay]->nextView = -1;
  myDisplays[activeDisplay]->secondaryActiveView = mySettings->getSecondaryActiveView();

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
  if (digitalRead(PIN_LEFT_KEY) == LOW) { // LEFT KEY PRESSED

    debug->println(DEBUG_LEVEL_DEBUG, "Left key pressed");
    
    activeDisplay--;
    if (activeDisplay < 1) {
      activeDisplay = MAX_DISPLAYS;
    }

  } else if (digitalRead(PIN_RIGHT_KEY) == LOW) {

    debug->println(DEBUG_LEVEL_DEBUG, "Right key pressed");
    
    activeDisplay++;
    if (activeDisplay > MAX_DISPLAYS) {
      activeDisplay = 1;
    }

  } else if (digitalRead(PIN_ENTER_KEY) == LOW) { // ENTER KEY PRESSED

    debug->println(DEBUG_LEVEL_DEBUG, "Enter key pressed");
    
  } else if (digitalRead(PIN_UP_KEY) == LOW) { // UP KEY PRESSED

    debug->println(DEBUG_LEVEL_DEBUG, "Up key pressed");

    bool changeGauge = true;

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
    mySettings->setActiveView(myDisplays[1]->nextView);
    mySettings->setSecondaryActiveView(myDisplays[1]->secondaryActiveView);
    mySettings->save();

  } else if (digitalRead(PIN_DOWN_KEY) == LOW || (testDownKey && (millis() - lastTime) > TEST_KEY_DELAY)) { // DOWN KEY PRESSED

    bool changeGauge = true;

    if (testDownKey) {
      lastTime = millis();
      debug->println(DEBUG_LEVEL_DEBUG, "Test Down key");
    }
    debug->println(DEBUG_LEVEL_DEBUG, "Down key pressed");

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
    mySettings->setActiveView(myDisplays[1]->nextView);
    mySettings->setSecondaryActiveView(myDisplays[1]->secondaryActiveView);
    mySettings->save();
  }
}

bool drawActiveGauge() {
  
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
#endif

void loop() {
  
#ifdef USE_MULTI_THREAD
  //vTaskDelete(NULL);
  vTaskDelay(DELAY_MAIN_TASK / portTICK_PERIOD_MS);
#else

  debug->println(DEBUG_LEVEL_DEBUG2, "--- LOOP ---");

  viewIndex = myDisplays[activeDisplay]->activeView;
  viewId = myGauges[viewIndex]->getId();

  if (viewId != VIEW_NONE) {   
    
    if (odbAdapter->isDeviceConnected() && odbAdapter->isOBDConnected()) {

      bool changeView = drawActiveGauge();
      if (changeView) {
        debug->println(DEBUG_LEVEL_DEBUG, "Change view request");
        viewIndex = myDisplays[activeDisplay]->activeView;
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
          myGauges[viewIndex]->draw(changeView);
        }

      } else {
          debug->println(DEBUG_LEVEL_DEBUG2, "value NOT readed");
      }

      checkKeypad();

      delay(myGauges[viewId]->getInterval());

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

#endif
}


