#include <Arduino.h>
#include <WiFi.h>
#include <BluetoothSerial.h>
#include <ELMduino.h>

#include "defines.h"
#include "debug.h"
#include "gauge.h"
#include "displays.h"
#include "tasks.h"
#include "structs.h"

#ifdef ENABLE_EEPROM
  #include "settings.h"
  #include "myEEPROM.h"
#endif

#ifdef ENABLE_RTC_CLOCK
  #include "myRTC.h"
#endif

#ifdef ENABLE_OBD_BLUETOOTH
  #include "bluetoothOBD.h"
#endif

// TFT SPI
Adafruit_GC9A01A tft1(TFT1_CS, TFT1_DC);

#ifdef ENABLE_SECOND_DISPLAY
  Adafruit_GC9A01A tft2(TFT2_CS, TFT2_DC);
#endif

Debug debug;

ELM327 obd;
BluetoothSerial SerialBT;

S_DateTime dateTime;

char dateString[DATE_LENGTH];
char timeString[TIME_LENGTH];
char oldDateString[DATE_LENGTH];
char oldTimeString[TIME_LENGTH];

int activeDisplay;

// Thread handles
TaskHandle_t t_main_menu;
TaskHandle_t t_settings;
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

Displays *myDisplays[MAX_DISPLAYS];
Gauge *myGauges[MAX_VIEWS + 1];
Settings mySettings;

#ifdef ENABLE_OBD_BLUETOOTH
  BluetoothOBD bluetoothOBD;
#endif

#ifdef ENABLE_EEPROM
  MyEEPROM myEEPROM(512);
#endif

#ifdef ENABLE_RTC_CLOCK
  RTC_DS1302 myRTC;
#endif

void setup() {

  // Initialize Serial and set debug level
  debug.start(115200, DEBUG_LEVEL_DEBUG);

  debug.println(DEBUG_LEVEL_INFO, F("Staring up..."));  

  btConnectedSemaphore = xSemaphoreCreateMutex();
  obdConnectedSemaphore = xSemaphoreCreateMutex();
  obdValueSemaphore = xSemaphoreCreateMutex();
  keyPadSemaphore = xSemaphoreCreateMutex();    

  #ifdef ENABLE_EEPROM
    myEEPROM.start();  
    
    if (myEEPROM.hasSignature()) {
      mySettings.load();
    } else {
      myEEPROM.createSignature();
      mySettings.setDefaults();
      mySettings.save();
    }
  #endif

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  debug.println(DEBUG_LEVEL_INFO, F("Staring up TFT1..."));
  tft1.begin();
  tft1.setRotation(0);
  tft1.fillScreen(BLACK);
  yield();

  #ifdef ENABLE_SECOND_DISPLAY
    debug.println(DEBUG_LEVEL_INFO, F("Staring up TFT2..."));
    tft2.begin();
    tft2.setRotation(0);
    tft2.fillScreen(BLACK);
    yield();
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
  
  debug.println(DEBUG_LEVEL_INFO, F("Creating gauges..."));
  

  myGauges[1] = new Gauge(myDisplays[1], VIEW_KPH, TYPE_GAUGE_GRAPH, DELAY_VIEW_KPH, (char*)"KM/h", (char*)"%d", WHITE, RED, false, false, 0, 0, 130, 180);
  
  myGauges[2] = new Gauge(myDisplays[1], VIEW_RPM, TYPE_GAUGE_GRAPH, DELAY_VIEW_RPM, (char*)"RPM", (char*)"%d", WHITE, RED, false, false, 0, 0, 6500, 7500);
  
  // Battery  
  myGauges[3] = new Gauge(myDisplays[1], VIEW_BATTERY_VOLTAGE, TYPE_GAUGE_GRAPH, DELAY_VIEW_BATTERY_VOLTAGE, (char*)"VOLT", (char*)"%0.1f", RED, RED, true, true, 110, 120, 140, 150);
  myGauges[3]->addSecondaryView(VIEW_BATTERY_VOLTAGE, VIEW_AMBIENT_TEMP, (char*)"%d C");
  myGauges[3]->addSecondaryView(VIEW_BATTERY_VOLTAGE, VIEW_BATTERY_VOLTAGE, (char*)"%0.1f V");
  myGauges[3]->addSecondaryView(VIEW_BATTERY_VOLTAGE, VIEW_AMBIENT_TEMP, (char*)"%d C");
  
  // Engine coolant  
  myGauges[4] = new Gauge(myDisplays[1], VIEW_COOLANT_TEMP, TYPE_GAUGE_GRAPH, DELAY_VIEW_COOLANT_TEMP, (char*)"ENGINE", (char*)"%d C", BLUE, RED, true, true, 20, 40, 105, 120);
  myGauges[4]->addSecondaryView(VIEW_COOLANT_TEMP, VIEW_AMBIENT_TEMP, (char*)"%d C");
  myGauges[4]->addSecondaryView(VIEW_COOLANT_TEMP, VIEW_BATTERY_VOLTAGE, (char*)"%0.1f V");
  
  // Ambient  
  myGauges[5] = new Gauge(myDisplays[1], VIEW_AMBIENT_TEMP, TYPE_DUAL_TEXT, DELAY_VIEW_AMBIENT_AIR_TEMP, (char*)"OUT", (char*)"%d C", BLUE, WHITE, true, false, -20, 3, 50, 50);
  myGauges[5]->addSecondaryView(VIEW_AMBIENT_TEMP, VIEW_INTAKE_TEMP, (char*)"%d C");
  myGauges[5]->addSecondaryView(VIEW_AMBIENT_TEMP, VIEW_BATTERY_VOLTAGE, (char*)"%0.1f V");
  
  // Intake (+Ambient)
  
  myGauges[6] = new Gauge(myDisplays[1], VIEW_INTAKE_TEMP, TYPE_GAUGE_GRAPH, DELAY_VIEW_INTAKE_AIR_TEMP, (char*)"INTAKE", (char*)"%d C", WHITE, RED, false, true, -20, -20, 65, 100);
  myGauges[6]->addSecondaryView(VIEW_INTAKE_TEMP, VIEW_AMBIENT_TEMP, (char*)"%d C");
  myGauges[6]->addSecondaryView(VIEW_INTAKE_TEMP, VIEW_BATTERY_VOLTAGE, (char*)"%0.1f V");
  
  myGauges[7] = new Gauge(myDisplays[1], VIEW_TIMING_ADV, TYPE_GAUGE_GRAPH, DELAY_VIEW_ADV, (char*)"ADVANCE", (char*)"%d º", RED, WHITE, false, false, 0, 0, 360, 360);

  //myGauges[8] = new Gauge(myDisplays[1], VIEW_DATE_TIME, TYPE_DATE, DELAY_VIEW_DATE_TIME, (char*)"  ", (char*)"  ", 0, 0, false, false, 0, 0, 0, 0);

  activeDisplay = 1;
  myDisplays[1]->activeView = 0;
  myDisplays[1]->nextView = 1;
  myDisplays[1]->secondaryActiveView = 0;

  /*
  debug.println(DEBUG_LEVEL_INFO, F("Staring up MainMenu"));
  xTaskCreatePinnedToCore(
    main_menu_task, // Task function.
    "MainMenu",     // Name of task.
    10000,          // Stack size of task
    NULL,           // Parameter of the task
    0,              // Priority of the task
    &t_main_menu,   // Task handle to keep track of created task
    0);             // Pin task to core 0

  delay(200);*/

  debug.println(DEBUG_LEVEL_INFO, F("Staring up OBD Manager..."));

  xTaskCreatePinnedToCore(
    obd_task,       // Task function.
    "OBD_Manager",  // Name of task.
    10000,          // Stack size of task
    NULL,           // Parameter of the task
    0,              // Priority of the task
    &t_core1_obd,   // Task handle to keep track of created task
    1);             // Pin task to core 1

  delay(500);

  debug.println(DEBUG_LEVEL_INFO, F("Staring up View Manager 1..."));

  xTaskCreatePinnedToCore(
    tft1_task,        // Task function.
    "View_Manager1",  // Name of task.
    10000,            // Stack size of task
    NULL,             // Parameter of the task
    0,                // Priority of the task
    &t_core0_tft1,    // Task handle to keep track of created task
    0);               // Pin task to core 0

  delay(200);
 

  debug.println(DEBUG_LEVEL_INFO, F("Staring up Keypad Manager..."));

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

  debug.println(DEBUG_LEVEL_INFO, F("Setup completed"));
}

void loop() {
  //vTaskDelete(NULL);
  vTaskDelay(DELAY_MAIN_TASK / portTICK_PERIOD_MS);
}


