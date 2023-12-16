#include <Arduino.h>
#include "defines.h"

#ifdef ENABLE_OBD_BLUETOOTH

#include <BluetoothSerial.h>
#include <ELMduino.h>

#include "bluetoothOBD.h"

#include "debug.h"
#include "vars.h"

BluetoothOBD::BluetoothOBD(String obdDeviceName, String obdDeviceAddr) {

    debug->println(DEBUG_LEVEL_DEBUG, "::BluetoothOBD activated");

    this->obdDeviceName = obdDeviceName;
    this->obdDeviceAddr = obdDeviceAddr;

    sec_mask = ESP_SPP_SEC_NONE; // or ESP_SPP_SEC_ENCRYPT|ESP_SPP_SEC_AUTHENTICATE to request pincode confirmation
    role = ESP_SPP_ROLE_SLAVE;

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

/*void BluetoothOBD::callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    debug->println(DEBUG_LEVEL_INFO, "BluetoothOBD Client Connected");
  }

  if (event == ESP_SPP_CLOSE_EVT) {
    debug->println(DEBUG_LEVEL_INFO, "BluetoothOBD Client disconnected");
  }
}*/

void BluetoothOBD::disconnect() {
    if (SerialBT.connected()) {
        SerialBT.disconnect();        
    }
    setBtConnected(false);
    setObdConnected(false);
}

bool BluetoothOBD::connect(char *pin) {
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
    debug->println(DEBUG_LEVEL_INFO, obdDeviceName.c_str());

    //connect to obd2
    if (scanBTdevice()) {
        
        String txt = "";        
        for (int i=1; i<4; i++) {

            txt = "Connecting to " + obdDeviceName + " - " + ByteArraytoString(client_addr) + " (" + String(i) + ")...";
            debug->println(DEBUG_LEVEL_INFO, txt.c_str());
            connected = SerialBT.connect(client_addr, 0, sec_mask, role);

            if (connected) {
                break;
            } else {
                delay(1000);
            }
        }
    } else {
        debug->println(DEBUG_LEVEL_ERROR, "OBDII Adaptor not found!");
    }
    
    if (connected) {
        debug->println(DEBUG_LEVEL_INFO, "Bluetooth connected succesfully!");

        setBtConnected(true);

        bool obdReady = false;
        obd = new ELM327();
        
        for (int i=1; i<4; i++) {
            debug->print(DEBUG_LEVEL_INFO, "Connecting to OBD (");
            debug->print(DEBUG_LEVEL_INFO, i);
            debug->println(DEBUG_LEVEL_INFO, ")...");

            obdReady = obd->begin(SerialBT, OBD_DEBUG_LOGS, ODB_TIMEOUT_MS);
            if (obdReady) {
                break;
            } else {
                delay(1000);
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

//convert bt address to text
//{0x00,0x1d,0xa5,0x00,0x12,0x92} -> 00:1d:a5:00:12:92
String BluetoothOBD::ByteArraytoString(esp_bd_addr_t bt_address) {
  String txt = "";
  String nib = "";
    for (int i=0; i<ESP_BD_ADDR_LEN+1; i++) {
       nib = String(bt_address[i], HEX);
       if (nib.length() < 2) 
         nib = "0" + nib;
       txt = txt + nib + ":";
    }
    nib = String(bt_address[ESP_BD_ADDR_LEN + 1], HEX);
    if (nib.length() < 2) nib = "0" + nib;
    txt = txt + nib;
    return txt;
}

bool BluetoothOBD::scanBTdevice() {

    debug->println(DEBUG_LEVEL_INFO, "Scanning for OBDII Adapter...");

    bool foundOBD2 = false;

    // BTScanResults* btDeviceList = BTSerial.getScanResults();  // maybe accessing from different threads!
    if (SerialBT.discoverAsync([](BTAdvertisedDevice* pDevice) {
            // BTAdvertisedDeviceSet*set = reinterpret_cast<BTAdvertisedDeviceSet*>(pDevice);
            // btDeviceList[pDevice->getAddress()] = * set;
            //String txt = pDevice->toString().c_str();
            
            debug->print(DEBUG_LEVEL_INFO, "Found device: ");
            debug->println(DEBUG_LEVEL_INFO, pDevice->toString().c_str());
            bluetoothOBD->setDeviceName(pDevice->getName().c_str());
            bluetoothOBD->setDeviceAddress(pDevice->getAddress().toString().c_str());
            })) 
    {
        delay(DELAY_BT_DISCOVER_TIME);
        SerialBT.discoverAsyncStop();
        debug->println(DEBUG_LEVEL_INFO, "Scan completed");
        delay(1000);
    
    } else {
        debug->println(DEBUG_LEVEL_ERROR, "Error on discovering bluetooth clients.");
    }

    int index;
    int stringCount;

    //matching scan obd2 and config obd2

    String txt = deviceName + " - " + deviceAddr;
    debug->println(DEBUG_LEVEL_DEBUG, txt.c_str());

    if (deviceName == obdDeviceName || deviceAddr == obdDeviceAddr) {//match name.
        //00:1d:a5:00:12:92 -> {0x00,0x1d,0xa5,0x00,0x12,0x92};
        //copy match bt mac address to client_name to connect
        stringCount = 0;
        while (deviceAddr.length() > 0)
        {
            index = deviceAddr.indexOf(':');
            if (index == -1)  {// No : found
                client_addr[stringCount] = strtol(deviceAddr.c_str(), 0, 16);//convert hex string to byte
                break;
            } else {
                client_addr[stringCount] = strtol(deviceAddr.substring(0, index).c_str(),0,16); //convert hex string to byte
                stringCount++;
                deviceAddr = deviceAddr.substring(index + 1);
            }
        }
        foundOBD2 = true;
    }
    
    return foundOBD2;
}

void BluetoothOBD::setDeviceName(String deviceName) {
    this->deviceName = deviceName;
}

void BluetoothOBD::setDeviceAddress(String deviceAddr) {
    this->deviceAddr = deviceAddr;
}

//supportedPIDs_1_20
int BluetoothOBD::getVoltage() {
    return this->voltage;
}
void BluetoothOBD::setVoltage(int voltage) {
    this->voltage = voltage;
}

int BluetoothOBD::getKph() {
    return this->kph;
}
void BluetoothOBD::setKph(int kph) {
    this->kph = kph;
}

int BluetoothOBD::getRpm() {
    return this->rpm;
}
void BluetoothOBD::setRpm(int rpm) {
    this->rpm = rpm;
}

int BluetoothOBD::getCoolantTemp() {
    return this->coolantTemp;
}
void BluetoothOBD::setCoolantTemp(int coolantTemp) {
    this->coolantTemp = coolantTemp;
}

int BluetoothOBD::getIntakeTemp() {
    return this->intakeTemp;
}
void BluetoothOBD::setIntakeTemp(int intakeTemp) {
    this->intakeTemp = intakeTemp;
}

int BluetoothOBD::getTimingAdvance() {
    return this->timingAdvance;
}
void BluetoothOBD::setTimingAdvance(int timingAdvance) {
    this->timingAdvance = timingAdvance;
}
int BluetoothOBD::getEngineLoad() {
    return this->engineLoad;
}
void BluetoothOBD::setEngineLoad(int engineLoad) {
    this->engineLoad = engineLoad;
}

int BluetoothOBD::getMafRate() {
    return this->mafRate;
}
void BluetoothOBD::setMafRate(int mafReate) {
    this->mafRate = mafRate;
}

int BluetoothOBD::getShortFuelTrim() {
    return this->shortFuelTrim;
}
void BluetoothOBD::setShortFuelTrim(int shortFuelTrim) {
    this->shortFuelTrim = shortFuelTrim;
}

int BluetoothOBD::getLongFuelTrim() {
    return this->longFuelTrim;
}
void BluetoothOBD::setLongFuelTrim(int longFuelTrim) {
    this->longFuelTrim = longFuelTrim;
}

int BluetoothOBD::getThrottle() {
    return this->throttle;
}
void BluetoothOBD::setThrottle(int throttle) {
    this->throttle = throttle;
}

//supportedPIDs_21_40
int BluetoothOBD::getFuelLevel() {
    return this->fuelLevel;
}
void BluetoothOBD::setFuelLevel(int fuelLevel) {
    this->fuelLevel = fuelLevel;
}

//supportedPIDs_41_60
int BluetoothOBD::getAmbientTemp() {
    return this->ambientTemp;
}
void BluetoothOBD::setAmbientTemp(int ambientTemp) {
    this->ambientTemp = ambientTemp;
}

int BluetoothOBD::getOilTemp() {
    return this->oilTemp;
}
void BluetoothOBD::setOilTemp(int oilTemp) {
    this->oilTemp = oilTemp;
}

int BluetoothOBD::getAbsLoad() {
    return this->absLoad;
}
void BluetoothOBD::setAbsLoad() {
    this->absLoad = absLoad;
}

#endif