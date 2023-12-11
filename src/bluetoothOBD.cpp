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
    btConnected = false;
    obdConnected = false;

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

bool BluetoothOBD::connect(char *OBDDeviceName, char *pin) {
  this->OBDDeviceName = OBDDeviceName;
  //SerialBT.register_callback(callback);
  bool ret = false;

  if (!SerialBT.begin("ESP32", true)) {
    debug.println(DEBUG_LEVEL_ERROR, F("An error occurred initializing Bluetooth"));
  } else {
    debug.println(DEBUG_LEVEL_INFO, F("Bluetooth initialized"));

    if (pin != nullptr) {
        SerialBT.setPin(pin);
    }

    bool connected;

    debug.print(DEBUG_LEVEL_INFO, F("Bluetooth Slave device: "));
    debug.println(DEBUG_LEVEL_INFO, OBDDeviceName);
    
    for (int i=1; i<11; i++) {

        debug.print(DEBUG_LEVEL_INFO, F("Connecting to Bluetooth ("));
        debug.print(DEBUG_LEVEL_INFO, i);
        debug.println(DEBUG_LEVEL_INFO, F(")..."));

        connected = SerialBT.connect(OBDDeviceName);
        if (connected) {
            break;
        } else {
            delay(3000);
        }
    }
    
    if (connected) {
        debug.println(DEBUG_LEVEL_INFO, F("Bluetooth connected succesfully!"));

        setBtConnected(true);

        bool obdReady = false;
        
        for (int i=1; i<11; i++) {
            debug.print(DEBUG_LEVEL_INFO, F("Connecting to OBD ("));
            debug.print(DEBUG_LEVEL_INFO, i);
            debug.println(DEBUG_LEVEL_INFO, F(")..."));

            obdReady = obd.begin(SerialBT, true, 3000);
            if (obdReady) {
                break;
            } else {
                delay(3000);
            }
        }
        
        if (obdReady) {
            setObdConnected(true);
            debug.print(DEBUG_LEVEL_INFO, F("Connected"));
        } else {
            debug.print(DEBUG_LEVEL_ERROR, F("Couldn't connect"));
        }
        debug.println(DEBUG_LEVEL_ERROR, F(" to ELM327"));
    } else {
        debug.println(DEBUG_LEVEL_INFO, F("Cannot connect to Bluetooth device"));
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