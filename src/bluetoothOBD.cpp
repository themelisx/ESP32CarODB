#include <Arduino.h>
#include "defines.h"

#ifdef ENABLE_OBD_BLUETOOTH

#include <BluetoothSerial.h>
#include <ELMduino.h>

#include "bluetoothOBD.h"

#include "debug.h"
#include "vars.h"

BluetoothOBD::BluetoothOBD() {

    debug->println(DEBUG_LEVEL_DEBUG, "::BluetoothOBD activated");

    voltage = INT_MIN;
    kph = INT_MIN;
    rpm = INT_MIN;
    coolantTemp = INT_MIN;
    ambientTemp = INT_MIN;
    intakeTemp = INT_MIN;
    timingAdvance = INT_MIN;
    btConnected = false;
    obdConnected = false;

}

void BluetoothOBD::setBtConnected(bool connected) {
    #ifdef USE_MULTI_THREAD
    xSemaphoreTake(btConnectedSemaphore, portMAX_DELAY);
    #endif
    this->btConnected = connected;
    #ifdef USE_MULTI_THREAD
    xSemaphoreGive(btConnectedSemaphore);
    #endif
}

void BluetoothOBD::setObdConnected(bool connected) {
    #ifdef USE_MULTI_THREAD
    xSemaphoreTake(obdConnectedSemaphore, portMAX_DELAY);
    #endif
    this->obdConnected = connected;
    #ifdef USE_MULTI_THREAD
    xSemaphoreGive(obdConnectedSemaphore);
    #endif
}

bool BluetoothOBD::isBluetoothConnected() {
    #ifdef USE_MULTI_THREAD
    xSemaphoreTake(btConnectedSemaphore, portMAX_DELAY);
    #endif
    bool isConnected = btConnected;
    #ifdef USE_MULTI_THREAD
    xSemaphoreGive(btConnectedSemaphore);
    #endif
    return isConnected;
}

bool BluetoothOBD::isOBDConnected() {
    #ifdef USE_MULTI_THREAD
    xSemaphoreTake(obdConnectedSemaphore, portMAX_DELAY);
    #endif
    bool isObdConnected = obdConnected;
    #ifdef USE_MULTI_THREAD
    xSemaphoreGive(obdConnectedSemaphore);
    #endif
    
    return isObdConnected;
}

/*
void BluetoothOBD::callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    debug->println("Client Connected");
  }

  if (event == ESP_SPP_CLOSE_EVT) {
    debug->println("Client disconnected");
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

bool BluetoothOBD::connect(char *OBDDeviceName, char *pin) {
  this->OBDDeviceName = OBDDeviceName;
  //SerialBT.register_callback(callback);
  bool ret = false;

  if (!SerialBT.begin("ESP32", true)) {
    debug->println(DEBUG_LEVEL_ERROR, "An error occurred initializing Bluetooth");
  } else {
    debug->println(DEBUG_LEVEL_INFO, "Bluetooth initialized");

    if (pin != nullptr) {
        SerialBT.setPin(pin);
    }

    bool connected;

    debug->print(DEBUG_LEVEL_INFO, "Bluetooth Slave device: ");
    debug->println(DEBUG_LEVEL_INFO, OBDDeviceName);
    
    for (int i=1; i<11; i++) {

        debug->print(DEBUG_LEVEL_INFO, "Connecting to Bluetooth (");
        debug->print(DEBUG_LEVEL_INFO, i);
        debug->println(DEBUG_LEVEL_INFO, ")...");

        connected = SerialBT.connect(OBDDeviceName);
        if (connected) {
            break;
        } else {
            delay(3000);
        }
    }
    
    if (connected) {
        debug->println(DEBUG_LEVEL_INFO, "Bluetooth connected succesfully!");

        setBtConnected(true);

        bool obdReady = false;
        
        for (int i=1; i<11; i++) {
            debug->print(DEBUG_LEVEL_INFO, "Connecting to OBD (");
            debug->print(DEBUG_LEVEL_INFO, i);
            debug->println(DEBUG_LEVEL_INFO, ")...");

            obdReady = obd->begin(SerialBT, true, 3000);
            if (obdReady) {
                break;
            } else {
                delay(3000);
            }
        }
        
        if (obdReady) {
            setObdConnected(true);
            debug->print(DEBUG_LEVEL_INFO, "Connected");
        } else {
            debug->print(DEBUG_LEVEL_ERROR, "Couldn't connect");
        }
        debug->println(DEBUG_LEVEL_ERROR, " to ELM327");
    } else {
        debug->println(DEBUG_LEVEL_INFO, "Cannot connect to Bluetooth device");
    }
  }
  return ret;
}

int BluetoothOBD::getVoltage() {
    return this->voltage;
}

int BluetoothOBD::getKph() {
    return this->kph;
}

int BluetoothOBD::getRpm() {
    return this->rpm;
}

int BluetoothOBD::getCoolantTemp() {
    return this->coolantTemp;
}

int BluetoothOBD::getAmbientTemp() {
    return this->ambientTemp;
}

int BluetoothOBD::getIntakeTemp() {
    return this->intakeTemp;
}

int BluetoothOBD::getTimingAdvance() {
    return this->timingAdvance;
}

void BluetoothOBD::setVoltage(int voltage) {
    this->voltage = voltage;
}

void BluetoothOBD::setKph(int kph) {
    this->kph = kph;
}

void BluetoothOBD::setRpm(int rpm) {
    debug->println(DEBUG_LEVEL_DEBUG2, "set rpm value");
    this->rpm = rpm;
    debug->println(DEBUG_LEVEL_DEBUG2, "ok");
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