#include <Arduino.h>

#include "defines.h"

#ifdef USE_MULTI_THREAD

#include "debug.h"
#include "gauge.h"
#include "vars.h"

void obd_task(void *pvParameters) {
  debug->print(DEBUG_LEVEL_INFO, "OBD manager task running on core ");
  debug->println(DEBUG_LEVEL_INFO, xPortGetCoreID());

  Gauge *gauge;
  #ifdef ENABLE_SECOND_DISPLAY
  Gauge *gauge2;
  #endif

  #ifdef MOCK_OBD
    srand(time(NULL));
    odbAdapter->setDeviceConnected(true);
    odbAdapter->setObdConnected(true);
  #endif

  for (;;) {

    if (odbAdapter->isDeviceConnected() && odbAdapter->isOBDConnected()) {
      gauge = displayManager->getDisplay(1)->getActiveGauge();
      odbAdapter->updateOBDValue(gauge);
      #ifdef ENABLE_SECOND_DISPLAY
        gauge2 = displayManager->getDisplay(2)->getActiveGauge();
        odbAdapter->updateOBDValue(gauge2);
      #endif
      vTaskDelay(gauge->getInterval() / portTICK_PERIOD_MS);
    } else {
      #ifndef MOCK_OBD
        odbAdapter->connect(nullptr);
        vTaskDelay(DELAY_ODB / portTICK_PERIOD_MS);
      #endif
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