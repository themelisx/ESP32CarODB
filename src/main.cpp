#include <Arduino.h>
#include <WiFi.h>
#include <ELMduino.h>

#include "defines.h"
#include "debug.h"
#include "gauge.h"
#include "displays.h"
#include "structs.h"
#include "settings.h"

#ifdef ENABLE_OBD_BLUETOOTH
  #include <BluetoothSerial.h>
  #include "bluetoothOBD.h"
#endif

#ifdef USE_MULTI_THREAD
  #include "tasks.h"
#endif

#ifdef ENABLE_EEPROM
  #include "myEEPROM.h"
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
SemaphoreHandle_t keyPadSemaphore;
SemaphoreHandle_t btConnectedSemaphore;
SemaphoreHandle_t obdConnectedSemaphore;
SemaphoreHandle_t obdValueSemaphore;
#endif

#ifdef ENABLE_OBD_BLUETOOTH
  BluetoothSerial SerialBT;
  BluetoothOBD *bluetoothOBD;
  
#endif

#ifdef ENABLE_EEPROM
  MyEEPROM myEEPROM(512);
#endif

#ifdef ENABLE_RTC_CLOCK
  RTC_DS1302 myRTC;
#endif

Debug *debug;
ELM327 *obd;

S_DateTime dateTime;

char dateString[DATE_LENGTH];
char timeString[TIME_LENGTH];
char oldDateString[DATE_LENGTH];
char oldTimeString[TIME_LENGTH];

int activeDisplay;

Displays *myDisplays[MAX_DISPLAYS + 1];
Gauge *myGauges[MAX_VIEWS + 1];
Settings *mySettings;

bool testDownKey = true;
  
int newValue = INT_MIN;
bool changeView = true;
bool shouldCheck = true;
int runs = 0;

int viewId;
unsigned long lastTime;

void setup() {

  // Initialize Serial and set debug level
  //debug.start(115200, DEBUG_LEVEL_INFO);
  debug = new Debug();
  debug->start(115200, DEBUG_LEVEL_INFO);

  debug->println(DEBUG_LEVEL_INFO, "Staring up...");

#ifdef USE_MULTI_THREAD
  btConnectedSemaphore = xSemaphoreCreateMutex();
  obdConnectedSemaphore = xSemaphoreCreateMutex();
  obdValueSemaphore = xSemaphoreCreateMutex();
  keyPadSemaphore = xSemaphoreCreateMutex();

  xSemaphoreGive(btConnectedSemaphore);
  xSemaphoreGive(obdConnectedSemaphore);
  xSemaphoreGive(obdValueSemaphore);
  xSemaphoreGive(keyPadSemaphore);
#endif

  mySettings = new Settings();
  mySettings->load();

  #ifdef ENABLE_OBD_BLUETOOTH
    //bluetoothOBD = new BluetoothOBD("OBDII", "11:22:33:dd:ee:ff");
    bluetoothOBD = new BluetoothOBD("OBDII", "00:12:6f:10:24:aa");
  #endif

  debug->println(DEBUG_LEVEL_INFO, "Disabling WiFi...");  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

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
  

  myGauges[1] = new Gauge(myDisplays[1], VIEW_KPH, TYPE_GAUGE_GRAPH, DELAY_VIEW_KPH, (char*)"Km/h", (char*)"%d", WHITE, RED, false, false, 0, 0, 130, 200);
  myGauges[1]->addSecondaryView(VIEW_KPH, VIEW_RPM, (char*)"%d");
  myGauges[1]->addSecondaryView(VIEW_KPH, VIEW_AMBIENT_TEMP, (char*)"O: %d C");
  myGauges[1]->addSecondaryView(VIEW_KPH, VIEW_INTAKE_TEMP, (char*)"I: %d C");
  myGauges[1]->addSecondaryView(VIEW_KPH, VIEW_COOLANT_TEMP, (char*)"E: %d C");
  
  myGauges[2] = new Gauge(myDisplays[1], VIEW_RPM, TYPE_GAUGE_GRAPH, DELAY_VIEW_RPM, (char*)"RPM", (char*)"%d", WHITE, RED, false, false, 0, 0, 6500, 7500);
  myGauges[2]->addSecondaryView(VIEW_RPM, VIEW_KPH, (char*)"%d");
  myGauges[2]->addSecondaryView(VIEW_RPM, VIEW_AMBIENT_TEMP, (char*)"O: %d C");
  myGauges[2]->addSecondaryView(VIEW_RPM, VIEW_INTAKE_TEMP, (char*)"I: %d C");
  myGauges[2]->addSecondaryView(VIEW_RPM, VIEW_COOLANT_TEMP, (char*)"E: %d C");
  
  // Battery  
  myGauges[3] = new Gauge(myDisplays[1], VIEW_BATTERY_VOLTAGE, TYPE_GAUGE_GRAPH, DELAY_VIEW_BATTERY_VOLTAGE, (char*)"Volt", (char*)"%0.1f", RED, RED, true, true, 110, 120, 140, 150);
  myGauges[3]->addSecondaryView(VIEW_BATTERY_VOLTAGE, VIEW_AMBIENT_TEMP, (char*)"O: %d C");
  myGauges[3]->addSecondaryView(VIEW_BATTERY_VOLTAGE, VIEW_INTAKE_TEMP, (char*)"I: %d C");
  myGauges[3]->addSecondaryView(VIEW_BATTERY_VOLTAGE, VIEW_COOLANT_TEMP, (char*)"E: %d C");
  
  // Engine coolant  
  myGauges[4] = new Gauge(myDisplays[1], VIEW_COOLANT_TEMP, TYPE_GAUGE_GRAPH, DELAY_VIEW_COOLANT_TEMP, (char*)"Engine", (char*)"%d C", BLUE, RED, true, true, 0, 40, 105, 120);
  myGauges[4]->addSecondaryView(VIEW_COOLANT_TEMP, VIEW_AMBIENT_TEMP, (char*)"O: %d C");
  myGauges[4]->addSecondaryView(VIEW_COOLANT_TEMP, VIEW_INTAKE_TEMP, (char*)"I: %d C");
  myGauges[4]->addSecondaryView(VIEW_COOLANT_TEMP, VIEW_BATTERY_VOLTAGE, (char*)"%0.1f V");
  
  // Ambient  
  myGauges[5] = new Gauge(myDisplays[1], VIEW_AMBIENT_TEMP, TYPE_DUAL_TEXT, DELAY_VIEW_AMBIENT_AIR_TEMP, (char*)"Out", (char*)"%d C", BLUE, WHITE, true, false, -20, 3, 50, 50);
  myGauges[5]->addSecondaryView(VIEW_AMBIENT_TEMP, VIEW_INTAKE_TEMP, (char*)"I: %d C");
  myGauges[5]->addSecondaryView(VIEW_AMBIENT_TEMP, VIEW_COOLANT_TEMP, (char*)"E: %d C");
  myGauges[5]->addSecondaryView(VIEW_AMBIENT_TEMP, VIEW_BATTERY_VOLTAGE, (char*)"%0.1f V");
  
  // Intake (+Ambient)
  
  myGauges[6] = new Gauge(myDisplays[1], VIEW_INTAKE_TEMP, TYPE_GAUGE_GRAPH, DELAY_VIEW_INTAKE_AIR_TEMP, (char*)"Intake", (char*)"%d C", WHITE, RED, false, true, -20, -20, 65, 100);
  myGauges[6]->addSecondaryView(VIEW_INTAKE_TEMP, VIEW_AMBIENT_TEMP, (char*)"O: %d C");
  myGauges[6]->addSecondaryView(VIEW_INTAKE_TEMP, VIEW_COOLANT_TEMP, (char*)"E: %d C");
  myGauges[6]->addSecondaryView(VIEW_INTAKE_TEMP, VIEW_BATTERY_VOLTAGE, (char*)"%0.1f V");
  
  myGauges[7] = new Gauge(myDisplays[1], VIEW_TIMING_ADV, TYPE_GAUGE_GRAPH, DELAY_VIEW_ADV, (char*)"Advance", (char*)"%d º", RED, WHITE, false, false, 0, 0, 50, 50);

  //myGauges[8] = new Gauge(myDisplays[1], VIEW_DATE_TIME, TYPE_DATE, DELAY_VIEW_DATE_TIME, (char*)"  ", (char*)"  ", 0, 0, false, false, 0, 0, 0, 0);

  activeDisplay = 1;
  myDisplays[1]->activeView = mySettings->getActiveView();
  myDisplays[1]->nextView = -1;
  myDisplays[1]->secondaryActiveView = mySettings->getSecondaryActiveView();

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
    0);             // Pin task to core 1

  vTaskSuspend(t_core1_obd);

  debug->println(DEBUG_LEVEL_INFO, "Staring up Keypad Manager...");

  pinMode(PIN_UP_KEY, INPUT_PULLUP);
  pinMode(PIN_DOWN_KEY, INPUT_PULLUP);
  pinMode(PIN_LEFT_KEY, INPUT_PULLUP);
  pinMode(PIN_RIGHT_KEY, INPUT_PULLUP);
  pinMode(PIN_ENTER_KEY, INPUT_PULLUP);

  xTaskCreatePinnedToCore(
    keypad_task,      // Task function.
    "Keypad_Manager", // Name of task.
    10000,            // Stack size of task
    NULL,             // Parameter of the task
    0,                // Priority of the task
    &t_core0_keypad,  // Task handle to keep track of created task
    0);               // Pin task to core 0

  vTaskSuspend(t_core0_keypad);

  debug->println(DEBUG_LEVEL_INFO, "Setup completed\nStarting tasks...");

  vTaskResume(t_core0_tft1);  
  delay(1000);

  vTaskResume(t_core0_keypad);
  delay(1000);

  debug->println(DEBUG_LEVEL_INFO, "Starting OBD...");
  vTaskResume(t_core1_obd);
#else

  #ifdef ENABLE_OBD_BLUETOOTH
    #ifndef MOCK_OBD
      // if device does not have pin use the follow
      bluetoothOBD->connect(nullptr);
      //bool connected = bluetoothOBD->connect(OBD_DEVICE_PIN);
    #else
      bluetoothOBD->setBtConnected(true);
      bluetoothOBD->setObdConnected(true);
    #endif
  #endif

  pinMode(PIN_UP_KEY, INPUT_PULLUP);
  pinMode(PIN_DOWN_KEY, INPUT_PULLUP);
  pinMode(PIN_LEFT_KEY, INPUT_PULLUP);
  pinMode(PIN_RIGHT_KEY, INPUT_PULLUP);
  pinMode(PIN_ENTER_KEY, INPUT_PULLUP);

  debug->println(DEBUG_LEVEL_INFO, "Setup completed");

  lastTime = millis();
#endif

}

void readObdValue(int activeViewId) {

  int newValue;

  switch (activeViewId) {
    case VIEW_BATTERY_VOLTAGE: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_batteryVoltage;
          #else
            newValue = obd->batteryVoltage() * 10;
          #endif
          break;
    case VIEW_KPH:
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_kph;
          #else
            newValue = obd->kph(); 
          #endif
          break;
    case VIEW_RPM: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_rpm;
          #else
            newValue = obd->rpm(); 
          #endif
          break;
    case VIEW_COOLANT_TEMP: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_engineCoolantTemp;
          #else
            newValue = obd->engineCoolantTemp(); 
          #endif
          break;
    //case VIEW_OIL_TEMP: newValue = obd->oilTemp(); break;
    case VIEW_AMBIENT_TEMP:
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_ambientAirTemp;
          #else
            newValue = obd->ambientAirTemp(); 
          #endif 
          break;
    case VIEW_INTAKE_TEMP:
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_intakeAirTemp;
          #else
            newValue = obd->intakeAirTemp(); 
          #endif
          break;
    case VIEW_TIMING_ADV: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_timingAdvance;
          #else
            newValue = obd->timingAdvance(); 
          #endif
          break;
    case VIEW_NONE:
          debug->println(DEBUG_LEVEL_INFO, "Inactive view");
          break;
    default:
          debug->print(DEBUG_LEVEL_ERROR, activeViewId);
          debug->println(DEBUG_LEVEL_ERROR, " is an unknown view");
  }

  #ifdef MOCK_OBD
    bool saveValue = true;
  #else
    bool saveValue = false;
    while (1)
    if (obd->nb_rx_state == ELM_SUCCESS) {
      saveValue = true;
      break;
    } else if (obd->nb_rx_state == ELM_GETTING_MSG) {
      delay(DELAY_READING);
    } else {      
      debug->println(DEBUG_LEVEL_INFO, "OBD Read error");
      break;
    }
  #endif

  #ifdef ENABLE_OBD_BLUETOOTH
    if (saveValue) {
      switch (activeViewId) {
        case VIEW_BATTERY_VOLTAGE: 
              bluetoothOBD->setVoltage(newValue);
              break;
        case VIEW_KPH:
              bluetoothOBD->setKph(newValue);
              break;
        case VIEW_RPM: 
              bluetoothOBD->setRpm(newValue);
              break;
        case VIEW_COOLANT_TEMP: 
              bluetoothOBD->setCoolantTemp(newValue);
              break;
        case VIEW_AMBIENT_TEMP:
              bluetoothOBD->setAmbientTemp(newValue);
              break;
        case VIEW_INTAKE_TEMP:
              bluetoothOBD->setIntakeTemp(newValue);
              break;
        case VIEW_TIMING_ADV: 
              bluetoothOBD->setTimingAdvance(newValue);
              break;
        case VIEW_NONE:
              break;
        default:
              break;
      }
    }
  #endif
}

void loop() {
  
#ifdef USE_MULTI_THREAD
  //vTaskDelete(NULL);
  vTaskDelay(DELAY_MAIN_TASK / portTICK_PERIOD_MS);
#else

  debug->println(DEBUG_LEVEL_DEBUG2, "--- LOOP ---");
  
  viewId = myDisplays[activeDisplay]->activeView;
  newValue = INT_MIN;

  if (viewId != VIEW_NONE) {   

    #ifdef ENABLE_OBD_BLUETOOTH
      if (bluetoothOBD->isBluetoothConnected() && bluetoothOBD->isOBDConnected()) {

        debug->print(DEBUG_LEVEL_DEBUG2, "Getting info for gauge id: ");
        debug->println(DEBUG_LEVEL_DEBUG2, viewId);

        readObdValue(viewId);   
        if (myGauges[viewId]->secondaryViews.activeView != VIEW_NONE) {
          readObdValue(myGauges[viewId]->secondaryViews.activeView);
        }

        switch (viewId) {
          case VIEW_BATTERY_VOLTAGE: newValue = bluetoothOBD->getVoltage(); break;
          case VIEW_KPH: newValue = bluetoothOBD->getKph(); break;
          case VIEW_RPM: newValue = bluetoothOBD->getRpm(); break;
          case VIEW_COOLANT_TEMP: newValue = bluetoothOBD->getCoolantTemp(); break;
          case VIEW_AMBIENT_TEMP: newValue = bluetoothOBD->getAmbientTemp(); break;
          case VIEW_INTAKE_TEMP: newValue = bluetoothOBD->getIntakeTemp(); break;
          case VIEW_TIMING_ADV: newValue = bluetoothOBD->getTimingAdvance(); break;
          default: newValue = 0;
        }
      } else {
        #ifndef MOCK_OBD
          bluetoothOBD->connect(nullptr);
        #endif
      }
    #endif
    
    int oldValue = myGauges[viewId]->data.value;

    if (viewId == VIEW_DATE_TIME) {
      myGauges[viewId]->drawDateTime();
    } else if (changeView || oldValue != newValue) {
      //debug->print(DEBUG_LEVEL_INFO, "Readed value: ");
      //debug->println(DEBUG_LEVEL_INFO, newValue);
      //debug->println(DEBUG_LEVEL_INFO, "Drawing....");
      myGauges[viewId]->drawGauge(viewId, changeView, newValue);
    }
  }

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
      if (myDisplays[activeDisplay]->nextView == VIEW_DATE_TIME) {
        memset(oldDateString, 0, DATE_LENGTH);
        memset(oldTimeString, 0, TIME_LENGTH);
      }
    }
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
      if (myDisplays[activeDisplay]->nextView == VIEW_DATE_TIME) {
        memset(oldDateString, 0, DATE_LENGTH);
        memset(oldTimeString, 0, TIME_LENGTH);
      }
    }
    mySettings->save();
  }

  if (bluetoothOBD->isBluetoothConnected() && bluetoothOBD->isOBDConnected()) {
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

      changeView = true;

      myDisplays[activeDisplay]->getTFT()->fillScreen(BACK_COLOR);

      myGauges[viewId]->data.state = STATE_UNKNOWN;
      myGauges[viewId]->data.value = myGauges[viewId]->data.min;

      if (myGauges[viewId]->getType() == TYPE_GAUGE_GRAPH && myGauges[viewId]->secondaryViews.activeView == 0) {
        myGauges[viewId]->drawBorders();
      }
      delay(DELAY_VIEW_CHANGE);
    } else {
      changeView = false;
    }

    runs++;  

    if (!changeView) {
      delay(myGauges[viewId]->getInterval());
    }
  } else {
    myDisplays[activeDisplay]->printMsg("OBD...");
  }

#endif
}


