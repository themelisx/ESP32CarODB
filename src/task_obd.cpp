#include <Arduino.h>

#include "defines.h"

#ifdef USE_MULTI_THREAD

#include "debug.h"
#include "gauge.h"
#include "vars.h"
#include "displays.h"

void obd_task(void *pvParameters) {
  debug->print(DEBUG_LEVEL_INFO, "OBD manager task running on core ");
  debug->println(DEBUG_LEVEL_INFO, xPortGetCoreID());

  bool shouldCheck = true;
  int newValue;
  int runs;

  #ifndef MOCK_OBD
    // if device does not have pin use the follow
    bool connected = bluetoothOBD.connect(OBD_DEVICE_NAME, nullptr);
    //bool connected = bluetoothOBD.connect(OBD_DEVICE_NAME, OBD_DEVICE_PIN);
  #else
    bluetoothOBD.setBtConnected(true);
    bluetoothOBD.setObdConnected(true);
  #endif

  for (;;) {

    runs = 0;

    if (bluetoothOBD.isBluetoothConnected() && bluetoothOBD.isOBDConnected()) {
      for (int i = 1; i < MAX_DISPLAYS + 1; i++) {
        //if (myDisplays[activeDisplay].enabled) {
        runs++;
        newValue = INT_MIN;

        xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);
        int activeViewId = myDisplays[activeDisplay]->activeView;
        xSemaphoreGive(keyPadSemaphore);
        yield();

        debug->print(DEBUG_LEVEL_INFO, "Getting info for gauge id: ");
        debug->println(DEBUG_LEVEL_INFO, activeViewId);

        bool goToNext = false;
        do {
          shouldCheck = true;
          
          //xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);
          xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
          switch (activeViewId) {
            case VIEW_BATTERY_VOLTAGE: 
                  #ifdef MOCK_OBD
                    newValue = MOCK_OBD_batteryVoltage;
                  #else
                    newValue = obd.batteryVoltage() * 10;
                  #endif
                  bluetoothOBD.setVoltage(newValue);
                  break;
            case VIEW_KPH:
                  #ifdef MOCK_OBD
                    newValue = MOCK_OBD_kph;
                  #else
                    newValue = obd.kph(); 
                  #endif
                  bluetoothOBD.setKph(newValue);
                  break;
            case VIEW_RPM: 
                  #ifdef MOCK_OBD
                    newValue = MOCK_OBD_rpm;
                  #else
                    newValue = obd.rpm(); 
                  #endif
                  debug->println(DEBUG_LEVEL_DEBUG2, "set rpm");
                  bluetoothOBD.setRpm(newValue);
                  break;
            case VIEW_COOLANT_TEMP: 
                  #ifdef MOCK_OBD
                    newValue = MOCK_OBD_engineCoolantTemp;
                  #else
                    newValue = obd.engineCoolantTemp(); 
                  #endif
                  bluetoothOBD.setCoolantTemp(newValue);
                  break;
            //case VIEW_OIL_TEMP: newValue = obd.oilTemp(); break;
            case VIEW_AMBIENT_TEMP:
                  #ifdef MOCK_OBD
                    newValue = MOCK_OBD_ambientAirTemp;
                  #else
                    newValue = obd.ambientAirTemp(); 
                  #endif 
                  bluetoothOBD.setAmbientTemp(newValue);
                  break;
            case VIEW_INTAKE_TEMP:
                  #ifdef MOCK_OBD
                    newValue = MOCK_OBD_intakeAirTemp;
                  #else
                    newValue = obd.intakeAirTemp(); 
                  #endif
                  bluetoothOBD.setIntakeTemp(newValue);
                  break;
            case VIEW_TIMING_ADV: 
                  #ifdef MOCK_OBD
                    newValue = MOCK_OBD_timingAdvance;
                  #else
                    newValue = obd.timingAdvance(); 
                  #endif
                  bluetoothOBD.setTimingAdvance(newValue);
                  break;
            case VIEW_NONE:
              shouldCheck = false;
              debug->println(DEBUG_LEVEL_INFO, "Inactive view");
              break;
            default:
              shouldCheck = false;
              debug->print(DEBUG_LEVEL_INFO, activeViewId);
              debug->println(DEBUG_LEVEL_INFO, " is an unknown view");
          }
          xSemaphoreGive(obdValueSemaphore);
          //xSemaphoreGive(keyPadSemaphore);
          yield();

          debug->print(DEBUG_LEVEL_INFO, "New value: ");
          debug->println(DEBUG_LEVEL_INFO, newValue);

          if (shouldCheck) {
            #ifdef MOCK_OBD
              //xSemaphoreTake(keyPadSemaphore, portMAX_DELAY);
              xSemaphoreTake(myGauges[activeViewId]->semaphore, portMAX_DELAY);
              myGauges[activeViewId]->data.value = newValue;
              xSemaphoreGive(myGauges[activeViewId]->semaphore);
              //xSemaphoreGive(keyPadSemaphore);
              yield();
              goToNext = true;
              vTaskDelay(DELAY_ODB / portTICK_PERIOD_MS);
            #else
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
                debug->println(DEBUG_LEVEL_INFO, "OBD Read error");
              }
            #endif
          } else {
            goToNext = true;
          }
        } while (goToNext == false);
        //}
      }
    }

    if (runs == 0) {
      yield();
      vTaskDelay(DELAY_ODB / portTICK_PERIOD_MS);
    }
  }
}

#endif
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