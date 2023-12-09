#include <Arduino.h>
#include "defines.h"

#ifdef ENABLE_OBD_BLUETOOTH

#include <BluetoothSerial.h>
#include <ELMduino.h>

#include "bluetoothOBD.h"

#include "debug.h"
#include "vars.h"

BluetoothOBD::BluetoothOBD() {
    btConnectedSemaphore = xSemaphoreCreateMutex();
    obdConnectedSemaphore = xSemaphoreCreateMutex();

    voltage = INT_MIN;
    kph = INT_MIN;
    rpm = INT_MIN;
    coolantTemp = INT_MIN;
    ambientTemp = INT_MIN;
    intakeTemp = INT_MIN;
    timingAdvance = INT_MIN;
    
    setBtConnected(false);
    setObdConnected(false);
}

void BluetoothOBD::setBtConnected(bool connected) {
    xSemaphoreTake(btConnectedSemaphore, portMAX_DELAY);
    this->btConnected = connected;
    xSemaphoreGive(btConnectedSemaphore);
}

void BluetoothOBD::setObdConnected(bool connected) {
    xSemaphoreTake(obdConnectedSemaphore, portMAX_DELAY);
    this->obdConnected = connected;
    xSemaphoreGive(obdConnectedSemaphore);
}

bool BluetoothOBD::isBluetoothConnected() {
    xSemaphoreTake(btConnectedSemaphore, portMAX_DELAY);
    bool isConnected = btConnected;
    xSemaphoreGive(btConnectedSemaphore);
    return isConnected;
}

bool BluetoothOBD::isOBDConnected() {
    xSemaphoreTake(obdConnectedSemaphore, portMAX_DELAY);
    bool isObdConnected = obdConnected;
    xSemaphoreGive(obdConnectedSemaphore);
    return isObdConnected;
}

/*
void BluetoothOBD::callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    debug.println(F("Client Connected"));
  }

  if (event == ESP_SPP_CLOSE_EVT) {
    debug.println(F("Client disconnected"));
  }
}
*/

void BluetoothOBD::disconnect() {
    if (SerialBT.connected()) {
        SerialBT.disconnect();        
    }
    setBtConnected(false);
    setObdConnected(false);
}

bool BluetoothOBD::connect(char *OBDDeviceName, const char *pin) {
  this->OBDDeviceName = OBDDeviceName;
  //SerialBT.register_callback(callback);
  bool ret = false;

  if (!SerialBT.begin(OBDDeviceName)) {
    debug.println(DEBUG_LEVEL_ERROR, F("An error occurred initializing Bluetooth"));
  } else {
    debug.println(DEBUG_LEVEL_INFO, F("Bluetooth initialized"));

    bool connected;
    int count = 0;
    debug.print(DEBUG_LEVEL_INFO, F("Trying to connnect to "));
    debug.print(DEBUG_LEVEL_INFO, OBDDeviceName);
    do {
        debug.print(DEBUG_LEVEL_INFO, ".");
        connected = SerialBT.connect(OBDDeviceName);    
        delay(3000);
        count++;
        if (count > 10) break;

    } while (connected == false);
    
    if (connected) {
        debug.println(DEBUG_LEVEL_INFO, F("Bluetooth connected succesfully!"));

        xSemaphoreTake(btConnectedSemaphore, portMAX_DELAY);
        btConnected = true;
        xSemaphoreGive(btConnectedSemaphore);

        //obd.setPin(OBDPin);

        count = 0;
        bool obdReady = false;
        debug.print(DEBUG_LEVEL_INFO, F("Connecting to OBD"));
        do {
            obdReady = obd.begin(SerialBT, true, 3000);
            debug.print(DEBUG_LEVEL_INFO, F("."));
            if (count > 10) break;
        } while (obdReady == false);        
        
        if (obdReady) {
            xSemaphoreTake(obdConnectedSemaphore, portMAX_DELAY);
            obdConnected = true;
            xSemaphoreGive(obdConnectedSemaphore);
            debug.println(DEBUG_LEVEL_INFO, F("Connected to ELM327"));
        } else {
            debug.println(DEBUG_LEVEL_ERROR, F("Couldn't connect to OBD module"));
        }        
    } else {
        debug.println(DEBUG_LEVEL_INFO, F("Cannot connect to Bluetooth device"));
    }
  }
  return ret;
}

int BluetoothOBD::getVoltage() {
    return voltage;
}

int BluetoothOBD::getKph() {
    return kph;
}

int BluetoothOBD::getRpm() {
    return rpm;
}

int BluetoothOBD::getCoolantTemp() {
    return coolantTemp;
}

int BluetoothOBD::getAmbientTemp() {
    return ambientTemp;
}

int BluetoothOBD::getIntakeTemp() {
    return intakeTemp;
}

int BluetoothOBD::getTimingAdvance() {
    return timingAdvance;
}

void BluetoothOBD::setVoltage(int voltage) {
    this->voltage = voltage;
}

void BluetoothOBD::setKph(int kph) {
    this->kph = kph;
}

void BluetoothOBD::setRpm(int rpm) {
    this->rpm = rpm;
}

void BluetoothOBD::setCoolantTemp(int coolantTemp) {
    this->coolantTemp = coolantTemp;
}

void BluetoothOBD::setAmbientTemp(int ambientTemp) {
    this->ambientTemp = ambientTemp;
}

void BluetoothOBD::setIntakeTemp(int intakeTemp) {
    this->intakeTemp = intakeTemp;
}

void BluetoothOBD::setTimingAdvance(int timingAdvance) {
    this->timingAdvance = timingAdvance;
}

#endif