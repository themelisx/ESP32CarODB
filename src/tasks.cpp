#include <Arduino.h>

#include "defines.h"
#include "debug.h"
#include "gauge.h"
#include "vars.h"
#include "displays.h"

void saveToEEPROM() {
  #ifdef ENABLE_EEPROM
    if (!myEEPROM.hasSignature()) {
      myEEPROM.createSignature();
    }
    myEEPROM.writeByte(EEPROM_DISPLAY1_NEXT_VIEW, myDisplays[1]->nextView);
    myEEPROM.writeByte(EEPROM_DISPLAY1_SECONDARY_VIEW, myDisplays[1]->secondaryActiveView);
    #ifdef ENABLE_SECOND_DISPLAY
      myEEPROM.writeByte(EEPROM_DISPLAY2_NEXT_VIEW, myDisplays[2]->nextView);
      myEEPROM.writeByte(EEPROM_DISPLAY2_SECONDARY_VIEW, myDisplays[2]->secondaryActiveView);
    #endif
  #endif
}

void connectToOBD() {
  //SerialBT.register_callback(callback);

  if (!SerialBT.begin("ESP32", true)) {
    Serial.println(F("An error occurred initializing Bluetooth"));
  } else {
    Serial.println(("Bluetooth initialized"));
  }

  bool connected;
  do {
    Serial.print(("Trying to connnect to "));
    Serial.println(OBD_DEVICE_NAME);
    connected = SerialBT.connect(OBD_DEVICE_NAME);  
    vTaskDelay(2000 / portTICK_PERIOD_MS);  

  } while (connected == false);
  
  Serial.println(("Connected Succesfully!"));

  xSemaphoreTake(btConnectedSemaphore, portMAX_DELAY);
  btConnected = true;
  xSemaphoreGive(btConnectedSemaphore);

  //obd.setPin("1234");

  while (!obd.begin(SerialBT, true, 2000)) {
    Serial.println(F("Couldn't connect to OBD module"));
  }

  xSemaphoreTake(obdConnectedSemaphore, portMAX_DELAY);
  obdConnected = true;
  xSemaphoreGive(obdConnectedSemaphore);

  Serial.println(("Connected to ELM327"));
}




void keypad_task(void *pvParameters) {
  Serial.print("Keypad manager (Core 0) task running on core ");
  Serial.println(xPortGetCoreID());

  bool testDownKey = true;

  for (;;) {
    
    if (digitalRead(PIN_LEFT_KEY) == LOW) { // LEFT KEY PRESSED

      Serial.println(F("Left key pressed"));
      
      xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);
      activeDisplay--;
      if (activeDisplay < 1) {
        activeDisplay = MAX_DISPLAYS;
      }
      xSemaphoreGive(keyPadSemaphore);

    } else if (digitalRead(PIN_RIGHT_KEY) == LOW) {

      Serial.println(F("Right key pressed"));
      
      xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);
      activeDisplay++;
      if (activeDisplay > MAX_DISPLAYS) {
        activeDisplay = 1;
      }
      xSemaphoreGive(keyPadSemaphore);

    } else if (digitalRead(PIN_ENTER_KEY) == LOW) { // ENTER KEY PRESSED

      Serial.println(F("Enter key pressed"));
      
    } else if (digitalRead(PIN_UP_KEY) == LOW) { // UP KEY PRESSED

      Serial.println(F("Up key pressed"));

      bool changeGauge = true;
      xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);

      if (myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.count > 0) {
        myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.activeView--;

        if (myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.activeView > 0) {
          Serial.print(F("Changing to prev secondary view"));
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
      saveToEEPROM();
      xSemaphoreGive(keyPadSemaphore);

    } else if (digitalRead(PIN_DOWN_KEY) == LOW || testDownKey) { // DOWN KEY PRESSED

      bool changeGauge = true;

      Serial.println(F("Down key pressed"));
      xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);

      if (myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.count > 0) {
        myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.activeView++;

        if (myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.activeView <= myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.count) {
          Serial.print(F("Changing to next secondary view"));
          changeGauge = false;
        } else {
          Serial.print(F("Changing to next view"));
          if (myGauges[myDisplays[activeDisplay]->activeView]->getType() == TYPE_DUAL_TEXT) {
            Serial.print(F("view is dual text"));
            myGauges[myDisplays[activeDisplay]->activeView]->secondaryViews.activeView = 1;
          } else {
            Serial.print(F("view is NOT dual text"));
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
      saveToEEPROM();
      xSemaphoreGive(keyPadSemaphore);

      vTaskDelay(DELAY_VIEW_CHANGE / portTICK_PERIOD_MS);
    }

    vTaskDelay(DELAY_KEYPAD / portTICK_PERIOD_MS);
    if (testDownKey) {
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }    
  }
}




void tft1_task(void *pvParameters) {
  Serial.print(F("View manager (Core 0) task running on core "));
  Serial.println(xPortGetCoreID());

  int i = 1;
  int viewId = myDisplays[i]->nextView;

  int newValue;
  bool changeView = true;
  
  for (;;) {

    xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);

    if (myDisplays[i]->activeView != myDisplays[i]->nextView || 
        myDisplays[i]->secondaryActiveView != myGauges[viewId]->secondaryViews.activeView) {

      Serial.print(F("Changing Gauge at display "));
      Serial.println(i);

      myDisplays[i]->activeView = myDisplays[i]->nextView;
      myDisplays[i]->secondaryActiveView = myGauges[viewId]->secondaryViews.activeView;
      viewId = myDisplays[i]->nextView;

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

    xSemaphoreTake(myGauges[viewId]->semaphore, portMAX_DELAY);
    switch (viewId) {
      case VIEW_BATTERY_VOLTAGE: newValue = data.voltage; break;
      case VIEW_KPH: newValue = data.kph; break;
      case VIEW_RPM: newValue = data.rpm; break;
      case VIEW_COOLANT_TEMP: newValue = data.coolantTemp; break;
      case VIEW_AMBIENT_TEMP: newValue = data.ambientTemp; break;
      case VIEW_INTAKE_TEMP: newValue = data.intakeTemp; break;
      case VIEW_TIMING_ADV: newValue = data.timingAdvance; break;
      default: newValue = 0;
    }
    xSemaphoreGive(myGauges[viewId]->semaphore);

    if (viewId == VIEW_DATE_TIME) {
      myGauges[viewId]->drawDateTime();
    } else if (changeView || myGauges[viewId]->data.value != newValue) {
      //Serial.print("Readed value: ");
      //Serial.println(newValue);
      //Serial.println(F("Drawing...."));
      myGauges[viewId]->drawGauge(viewId, changeView, newValue);
    }

    vTaskDelay(myGauges[viewId]->getInterval() / portTICK_PERIOD_MS);
  }
}

void obd_task(void *pvParameters) {
  Serial.print(F("OBD manager (Core 1) task running on core "));
  Serial.println(xPortGetCoreID());

  data.kph = 0;
  data.rpm = 0;
  data.voltage = INT_MIN;
  data.ambientTemp = INT_MIN;
  data.coolantTemp = INT_MIN;
  data.intakeTemp = INT_MIN;
  data.timingAdvance = INT_MIN;
  //data.oilTemp = 35;

  bool bluetoothOk = false;
  bool obdOk = false;
  bool shouldCheck = true;
  int newValue;
  int runs;

  connectToOBD();

  for (;;) {

    xSemaphoreTake(btConnectedSemaphore, portMAX_DELAY);
    bluetoothOk = btConnected;
    xSemaphoreGive(btConnectedSemaphore);

    xSemaphoreTake(obdConnectedSemaphore, portMAX_DELAY);
    obdOk = obdConnected;
    xSemaphoreGive(obdConnectedSemaphore);

    runs = 0;

    if (bluetoothOk && obdOk) {
      for (int i = 1; i < MAX_DISPLAYS + 1; i++) {
        //if (myDisplays[activeDisplay].enabled) {
        runs++;
        newValue = INT_MIN;  //-2147483648

        xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);
        int activeViewId = myDisplays[activeDisplay]->activeView;
        xSemaphoreGive(keyPadSemaphore);
        bool goToNext = false;
        do {
          shouldCheck = true;
          switch (activeViewId) {
            case VIEW_BATTERY_VOLTAGE: newValue = obd.batteryVoltage() * 10; break;
            case VIEW_KPH: newValue = obd.kph(); break;
            case VIEW_RPM: newValue = obd.rpm(); break;
            case VIEW_COOLANT_TEMP: newValue = obd.engineCoolantTemp(); break;
            //case VIEW_OIL_TEMP: newValue = obd.oilTemp(); break;
            case VIEW_AMBIENT_TEMP: newValue = obd.ambientAirTemp(); break;
            case VIEW_INTAKE_TEMP: newValue = obd.intakeAirTemp(); break;
            case VIEW_TIMING_ADV: newValue = obd.timingAdvance(); break;
            case VIEW_NONE:
              shouldCheck = false;
              Serial.println(F("Inactive view"));
              break;
            default:
              shouldCheck = false;
              Serial.print(activeViewId);
              Serial.println(F(" is an unknown view"));
          }

          Serial.print("new value: ");
          Serial.println(newValue);

          if (shouldCheck) {
            if (obd.nb_rx_state == ELM_SUCCESS) {
              if (newValue != INT_MIN) {
                xSemaphoreTake(myGauges[activeViewId]->semaphore, portMAX_DELAY);
                myGauges[activeViewId]->data.value = newValue;
                xSemaphoreGive(myGauges[activeViewId]->semaphore);
              }
              goToNext = true;
              vTaskDelay(DELAY_ODB / portTICK_PERIOD_MS);
            } else if (obd.nb_rx_state == ELM_GETTING_MSG) {
              vTaskDelay(DELAY_READING / portTICK_PERIOD_MS);
            } else {
              goToNext = true;
              Serial.println(F("OBD Read error"));
            }
          } else {
            goToNext = true;
          }
        } while (goToNext == false);
        //}
      }
    }

    if (runs == 0) {
      vTaskDelay(DELAY_ODB / portTICK_PERIOD_MS);
    }
  }
}


/*
uint32_t supportedPIDs_1_20();

uint32_t monitorStatus();
uint16_t freezeDTC();
uint16_t fuelSystemStatus();
float engineLoad();
float engineCoolantTemp();
float shortTermFuelTrimBank_1();
float longTermFuelTrimBank_1();
float shortTermFuelTrimBank_2();
float longTermFuelTrimBank_2();
float fuelPressure();
uint8_t manifoldPressure();
float rpm();
int32_t kph();
float mph();
float timingAdvance();
float intakeAirTemp();
float mafRate();
float throttle();
uint8_t commandedSecAirStatus();
uint8_t oxygenSensorsPresent_2banks();
uint8_t obdStandards();
uint8_t oxygenSensorsPresent_4banks();
bool auxInputStatus();
uint16_t runTime();


uint32_t supportedPIDs_21_40();

uint16_t distTravelWithMIL();
float fuelRailPressure();
float fuelRailGuagePressure();
float commandedEGR();
float egrError();
float commandedEvapPurge();
float fuelLevel();
uint8_t warmUpsSinceCodesCleared();
uint16_t distSinceCodesCleared();
float evapSysVapPressure();
uint8_t absBaroPressure();
float catTempB1S1();
float catTempB2S1();
float catTempB1S2();
float catTempB2S2();


uint32_t supportedPIDs_41_60();

uint32_t monitorDriveCycleStatus();
float ctrlModVoltage();
float absLoad();
float commandedAirFuelRatio();
float relativeThrottle();
float ambientAirTemp();
float absThrottlePosB();
float absThrottlePosC();
float absThrottlePosD();
float absThrottlePosE();
float absThrottlePosF();
float commandedThrottleActuator();
uint16_t timeRunWithMIL();
uint16_t timeSinceCodesCleared();
float maxMafRate();
uint8_t fuelType();
float ethonolPercent();
float absEvapSysVapPressure();
float evapSysVapPressure2();
float absFuelRailPressure();
float relativePedalPos();
float hybridBatLife();
float oilTemp();
float fuelInjectTiming();
float fuelRate();
uint8_t emissionRqmts();


uint32_t supportedPIDs_61_80();

float demandedTorque();
float torque();
uint16_t referenceTorque();
uint16_t auxSupported();
Other commands
float batteryVoltage(void); // Gets vehicle battery voltage
int8_t get_vin_blocking(char vin[]); // Gets Vehicle Identification Number (VIN)
*/