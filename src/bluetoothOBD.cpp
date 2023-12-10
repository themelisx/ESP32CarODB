#include <Arduino.h>
#include "defines.h"

#ifdef ENABLE_OBD_BLUETOOTH

#include <BluetoothSerial.h>
#include <ELMduino.h>

#include "bluetoothOBD.h"

#include "debug.h"
#include "vars.h"

BluetoothOBD::BluetoothOBD() {

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

  SerialBT.setPin(pin);

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

        setBtConnected(true);

        count = 0;
        bool obdReady = false;
        debug.print(DEBUG_LEVEL_INFO, F("Connecting to OBD"));
        do {
            obdReady = obd.begin(SerialBT, true, 3000);
            debug.print(DEBUG_LEVEL_INFO, F("."));
            if (count > 10) break;
        } while (obdReady == false);        
        
        if (obdReady) {
            setObdConnected(true);
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
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    int ret = voltage;
    xSemaphoreGive(obdValueSemaphore);
    return ret;
}

int BluetoothOBD::getKph() {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    int ret = kph;
    xSemaphoreGive(obdValueSemaphore);
    return ret;
}

int BluetoothOBD::getRpm() {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    int ret = rpm;
    xSemaphoreGive(obdValueSemaphore);
    return ret;
}

int BluetoothOBD::getCoolantTemp() {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    int ret = coolantTemp;
    xSemaphoreGive(obdValueSemaphore);
    return ret;
}

int BluetoothOBD::getAmbientTemp() {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    int ret = ambientTemp;
    xSemaphoreGive(obdValueSemaphore);
    return ret;
}

int BluetoothOBD::getIntakeTemp() {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    int ret = intakeTemp;
    xSemaphoreGive(obdValueSemaphore);
    return ret;
}

int BluetoothOBD::getTimingAdvance() {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    int ret = timingAdvance;
    xSemaphoreGive(obdValueSemaphore);
    return ret;
}

void BluetoothOBD::setVoltage(int voltage) {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    this->voltage = voltage;
    xSemaphoreGive(obdValueSemaphore);
}

void BluetoothOBD::setKph(int kph) {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    this->kph = kph;
    xSemaphoreGive(obdValueSemaphore);
}

void BluetoothOBD::setRpm(int rpm) {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    this->rpm = rpm;
    xSemaphoreGive(obdValueSemaphore);
}

void BluetoothOBD::setCoolantTemp(int coolantTemp) {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    this->coolantTemp = coolantTemp;
    xSemaphoreGive(obdValueSemaphore);
}

void BluetoothOBD::setAmbientTemp(int ambientTemp) {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    this->ambientTemp = ambientTemp;
    xSemaphoreGive(obdValueSemaphore);
}

void BluetoothOBD::setIntakeTemp(int intakeTemp) {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    this->intakeTemp = intakeTemp;
    xSemaphoreGive(obdValueSemaphore);
}

void BluetoothOBD::setTimingAdvance(int timingAdvance) {
    xSemaphoreTake(obdValueSemaphore, portMAX_DELAY);
    this->timingAdvance = timingAdvance;
    xSemaphoreGive(obdValueSemaphore);
}

#endif