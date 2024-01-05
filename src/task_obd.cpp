#include <Arduino.h>

#include "defines.h"

#ifdef USE_MULTI_THREAD

#include "debug.h"
#include "gauge.h"
#include "vars.h"

void obd_task(void *pvParameters) {
  debug->print(DEBUG_LEVEL_INFO, "OBD manager task running on core ");
  debug->println(DEBUG_LEVEL_INFO, xPortGetCoreID());

  int viewId;
  int viewIndex;
  Gauge *gauge;

  #ifndef MOCK_OBD
    // if device does not have pin use the follow
    //odbAdapter->connect(nullptr);
    //bool connected = odbAdapter->connect(OBD_DEVICE_PIN);
  #else
    srand(time(NULL));
    odbAdapter->setDeviceConnected(true);
    odbAdapter->setObdConnected(true);
  #endif

  for (;;) {

    gauge = displayManager->getDisplay(displayManager->getActiveDisplayId())->getActiveGauge();

    if (odbAdapter->isDeviceConnected() && odbAdapter->isOBDConnected()) {
      debug->println(DEBUG_LEVEL_DEBUG, "Reading OBD...");

      bool valueReaded = odbAdapter->readValueForViewType(viewId);
      yield();

      if (valueReaded) {
        gauge->data.value = odbAdapter->getValueForViewType(viewId);
        debug->print(DEBUG_LEVEL_DEBUG, "Value readed: ");
        debug->println(DEBUG_LEVEL_DEBUG, gauge->data.value);
        yield();

        xSemaphoreTake(semaphoreActiveView, portMAX_DELAY);
        int secondaryViewIdx = gauge->secondaryViews.activeView;
        if (secondaryViewIdx != VIEW_NONE) {          
          int secondaryViewId = gauge->secondaryViews.ids[secondaryViewIdx];
          xSemaphoreGive(semaphoreActiveView);

          debug->print(DEBUG_LEVEL_DEBUG2, "Secondary Index: ");
          debug->println(DEBUG_LEVEL_DEBUG2, secondaryViewIdx);
          debug->print(DEBUG_LEVEL_DEBUG2, "Secondary ID: ");
          debug->println(DEBUG_LEVEL_DEBUG2, secondaryViewId);

          valueReaded = odbAdapter->readValueForViewType(secondaryViewId);
          yield();
          if (valueReaded) {
            gauge->secondaryViews.value[secondaryViewIdx] = odbAdapter->getValueForViewType(secondaryViewId);
            yield();
          } else {
            debug->println(DEBUG_LEVEL_DEBUG2, "secondary value NOT readed");
          }
        } else {
          xSemaphoreGive(semaphoreActiveView);
        }

      } else {
          debug->println(DEBUG_LEVEL_DEBUG2, "value NOT readed");
      }
    }
    vTaskDelay(gauge->getInterval() / portTICK_PERIOD_MS);
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