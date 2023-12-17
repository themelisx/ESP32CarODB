#include <Arduino.h>
#include "defines.h"

#ifdef USE_OBD_BLUETOOTH
    #include <BluetoothSerial.h>
#endif

#include <ELMduino.h>

#include "odbAdapter.h"

#include "debug.h"
#include "vars.h"

OdbAdapter::OdbAdapter(String deviceName, String deviceAddr) {

    debug->println(DEBUG_LEVEL_DEBUG, "::OdbAdapter activated");

    this->deviceName = deviceName;
    this->deviceAddr = deviceAddr;

    #ifdef USE_OBD_BLUETOOTH
        sec_mask = ESP_SPP_SEC_NONE; // or ESP_SPP_SEC_ENCRYPT|ESP_SPP_SEC_AUTHENTICATE to request pincode confirmation
        role = ESP_SPP_ROLE_SLAVE;
    #endif

    voltage = INT_MIN;
    kph = INT_MIN;
    rpm = INT_MIN;
    coolantTemp = INT_MIN;
    ambientTemp = INT_MIN;
    intakeTemp = INT_MIN;
    timingAdvance = INT_MIN;
    deviceConnected = false;
    obdConnected = false;

}

void OdbAdapter::setDeviceConnected(bool connected) {
    #ifdef USE_MULTI_THREAD
    xSemaphoreTake(btConnectedSemaphore, portMAX_DELAY);
    #endif
    this->deviceConnected = connected;
    #ifdef USE_MULTI_THREAD
    xSemaphoreGive(btConnectedSemaphore);
    #endif
}

void OdbAdapter::setObdConnected(bool connected) {
    #ifdef USE_MULTI_THREAD
    xSemaphoreTake(obdConnectedSemaphore, portMAX_DELAY);
    #endif
    this->obdConnected = connected;
    #ifdef USE_MULTI_THREAD
    xSemaphoreGive(obdConnectedSemaphore);
    #endif
}

bool OdbAdapter::isDeviceConnected() {
    #ifdef USE_MULTI_THREAD
    xSemaphoreTake(btConnectedSemaphore, portMAX_DELAY);
    #endif
    bool isConnected = deviceConnected;
    #ifdef USE_MULTI_THREAD
    xSemaphoreGive(btConnectedSemaphore);
    #endif
    return isConnected;
}

bool OdbAdapter::isOBDConnected() {
    #ifdef USE_MULTI_THREAD
    xSemaphoreTake(obdConnectedSemaphore, portMAX_DELAY);
    #endif
    bool isObdConnected = obdConnected;
    #ifdef USE_MULTI_THREAD
    xSemaphoreGive(obdConnectedSemaphore);
    #endif
    
    return isObdConnected;
}

/*void OdbAdapter::callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    debug->println(DEBUG_LEVEL_INFO, "OdbAdapter Client Connected");
  }

  if (event == ESP_SPP_CLOSE_EVT) {
    debug->println(DEBUG_LEVEL_INFO, "OdbAdapter Client disconnected");
  }
}*/

void OdbAdapter::disconnect() {
    if (SerialDevice.connected()) {
        SerialDevice.disconnect();        
    }
    setDeviceConnected(false);
    setObdConnected(false);
}

#ifdef USE_OBD_WIFI
bool OdbAdapter::connect(char *pin) {
{
    WiFi.mode(WIFI_AP);
    if (pin != nullptr) {
        WiFi.begin(ssid, pin);
    } else {
        WiFi.begin(ssid);
    }

    for (int i=1; i<4; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            deviceConnected = true;
            break;
        } else {
            delay(500);
        }        
    }

    if (deviceConnected) {
        debug->println(DEBUG_LEVEL_INFO, "Connecting to OBD over WiFi...");

        if (SerialDevice.connect(server, 35000)) {
            setObdConnected(true);
            debug->print(DEBUG_LEVEL_INFO, "Connected");
        } else {
            debug->print(DEBUG_LEVEL_ERROR, "Couldn't connect");
        }
        debug->println(DEBUG_LEVEL_ERROR, " to ELM327");

    } else {
        debug->println(DEBUG_LEVEL_ERROR, "OBDII Adaptor not found!");
    }
  }
}
#endif

#ifdef USE_OBD_BLUETOOTH
bool OdbAdapter::connect(char *pin) {
  //SerialDevice.register_callback(callback);
  bool ret = false;

  if (!SerialDevice.begin("ESP32", true)) {
    debug->println(DEBUG_LEVEL_ERROR, "An error occurred initializing Bluetooth");
  } else {
    debug->println(DEBUG_LEVEL_INFO, "Bluetooth initialized");

    if (pin != nullptr) {
        SerialDevice.setPin(pin);
    }

    bool connected;

    debug->print(DEBUG_LEVEL_INFO, "Bluetooth Slave device: ");
    debug->println(DEBUG_LEVEL_INFO, deviceName.c_str());

    //connect to obd2
    if (scanBTdevice()) {
        
        String txt = "";        
        for (int i=1; i<4; i++) {

            txt = "Connecting to " + deviceName + " - " + ByteArraytoString(client_addr) + " (" + String(i) + ")...";
            debug->println(DEBUG_LEVEL_INFO, txt.c_str());
            connected = SerialDevice.connect(client_addr, 0, sec_mask, role);

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
        debug->println(DEBUG_LEVEL_INFO, "Device connected succesfully!");

        setDeviceConnected(true);

        bool obdReady = false;
        obd = new ELM327();
        
        for (int i=1; i<4; i++) {
            debug->print(DEBUG_LEVEL_INFO, "Connecting to OBD (");
            debug->print(DEBUG_LEVEL_INFO, i);
            debug->println(DEBUG_LEVEL_INFO, ")...");

            obdReady = obd->begin(SerialDevice, OBD_DEBUG_LOGS, ODB_TIMEOUT_MS);
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
#endif

//convert bt address to text
//{0x00,0x1d,0xa5,0x00,0x12,0x92} -> 00:1d:a5:00:12:92
#ifdef USE_OBD_BLUETOOTH
String OdbAdapter::ByteArraytoString(esp_bd_addr_t bt_address) {
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
#endif

#ifdef USE_OBD_BLUETOOTH
bool OdbAdapter::scanBTdevice() {

    debug->println(DEBUG_LEVEL_INFO, "Scanning for OBDII Adapter...");

    foundOBD2 = false;
    //String tmpName = this->deviceName;

    // BTScanResults* btDeviceList = BTSerial.getScanResults();  // maybe accessing from different threads!
    if (SerialDevice.discoverAsync([](BTAdvertisedDevice* pDevice) {
            // BTAdvertisedDeviceSet*set = reinterpret_cast<BTAdvertisedDeviceSet*>(pDevice);
            // btDeviceList[pDevice->getAddress()] = * set;
            //String txt = pDevice->toString().c_str();
            
            debug->print(DEBUG_LEVEL_INFO, "Found device: ");
            debug->println(DEBUG_LEVEL_INFO, pDevice->toString().c_str());

            String tmpName = pDevice->getName().c_str();
            String tmpAddress = pDevice->getAddress().toString().c_str();

            if (tmpName == odbAdapter->deviceName || tmpAddress == odbAdapter->deviceAddr) {

                    odbAdapter->setFoundOBD2(true);
                    odbAdapter->setDeviceName(pDevice->getName().c_str());
                    odbAdapter->setDeviceAddress(pDevice->getAddress().toString().c_str());
            }
            
        })) 
    {
        delay(DELAY_BT_DISCOVER_TIME);
        SerialDevice.discoverAsyncStop();
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

    if (foundOBD2) {
        stringCount = 0;
        while (deviceAddr.length() > 0)
        {
            index = deviceAddr.indexOf(':');
            if (index == -1)  {
                client_addr[stringCount] = strtol(deviceAddr.c_str(), 0, 16);
                break;
            } else {
                client_addr[stringCount] = strtol(deviceAddr.substring(0, index).c_str(), 0, 16);
                stringCount++;
                deviceAddr = deviceAddr.substring(index + 1);
            }
        }
        foundOBD2 = true;
    }
    
    return foundOBD2;
}
#endif

bool OdbAdapter::readValueForViewType(int viewId) {

  bool valueReaded = false;
  int count = 0;

  debug->print(DEBUG_LEVEL_DEBUG2, "Getting info for gauge id: ");
  debug->println(DEBUG_LEVEL_DEBUG2, viewId);
  debug->println(DEBUG_LEVEL_DEBUG2, "Query ODB value");
  
  while (!valueReaded) {
    valueReaded = readObdValue(viewId); 
    count++;
    if (count > 200) {
      debug->println(DEBUG_LEVEL_DEBUG2, "Value not readed after 200 times");
      break;
    }
  }

  return valueReaded;
}

int OdbAdapter::getValueForViewType(int viewId) {
  int newValue;
  switch (viewId) {
    case VIEW_BATTERY_VOLTAGE: newValue = getVoltage(); break;
    case VIEW_KPH: newValue = getKph(); break;
    case VIEW_RPM: newValue = getRpm(); break;
    case VIEW_COOLANT_TEMP: newValue = getCoolantTemp(); break;            
    case VIEW_INTAKE_TEMP: newValue = getIntakeTemp(); break;
    case VIEW_TIMING_ADV: newValue = getTimingAdvance(); break;
    case VIEW_ENGINE_LOAD: newValue = getEngineLoad(); break;    
    case VIEW_SHORT_FUEL_TRIM: newValue = getShortFuelTrim(); break;
    case VIEW_LONG_FUEL_TRIM: newValue = getLongFuelTrim(); break;
    case VIEW_THROTTLE: newValue = getThrottle(); break;
    case VIEW_MAF_RATE: newValue = getMafRate(); break;
    //supportedPIDs_21_40
    case VIEW_FUEL_LEVEL: newValue = getFuelLevel(); break;
    //supportedPIDs_41_60
    case VIEW_AMBIENT_TEMP: newValue = getAmbientTemp(); break;
    case VIEW_OIL_TEMP: newValue = getOilTemp(); break;
    case VIEW_ABS_LOAD: newValue = getAbsLoad(); break;
    default: newValue = INT_MIN;
  }
  return newValue;
}

void OdbAdapter::setValueForViewType(int viewTypeId, int newValue) {
  switch (viewTypeId) {
    case VIEW_BATTERY_VOLTAGE: setVoltage(newValue); break;
    case VIEW_KPH: setKph(newValue); break;
    case VIEW_RPM: setRpm(newValue); break;
    case VIEW_COOLANT_TEMP: setCoolantTemp(newValue); break;        
    case VIEW_INTAKE_TEMP: setIntakeTemp(newValue); break;
    case VIEW_TIMING_ADV: setTimingAdvance(newValue); break;
    case VIEW_ENGINE_LOAD: setEngineLoad(newValue); break;    
    case VIEW_SHORT_FUEL_TRIM: setShortFuelTrim(newValue); break;
    case VIEW_LONG_FUEL_TRIM: setLongFuelTrim(newValue); break;
    case VIEW_THROTTLE: setThrottle(newValue); break;
    case VIEW_MAF_RATE: setMafRate(newValue); break;
    //supportedPIDs_21_40
    case VIEW_FUEL_LEVEL: setFuelLevel(newValue); break;
    //supportedPIDs_41_60
    case VIEW_AMBIENT_TEMP: setAmbientTemp(newValue); break;
    case VIEW_OIL_TEMP: setOilTemp(newValue); break;
    case VIEW_ABS_LOAD: setAbsLoad(newValue); break;
    case VIEW_NONE: break;    
    default: break;
  }
}

bool OdbAdapter::readObdValue(int viewTypeId) {

  int newValue = 0;
  bool doAction = true;

  switch (viewTypeId) {
    case VIEW_BATTERY_VOLTAGE: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_batteryVoltage;
          #else
            newValue = int(obd->batteryVoltage() * 10);
          #endif
          break;
    case VIEW_KPH:
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_kph;
          #else
            newValue = (int)obd->kph(); 
          #endif
          break;
    case VIEW_RPM: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_rpm;
          #else
            newValue = (int)obd->rpm(); 
          #endif
          break;
    case VIEW_COOLANT_TEMP: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_engineCoolantTemp;
          #else
            newValue = (int)obd->engineCoolantTemp(); 
          #endif
          break;    
    case VIEW_INTAKE_TEMP:
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_intakeAirTemp;
          #else
            newValue = (int)obd->intakeAirTemp(); 
          #endif
          break;
    case VIEW_TIMING_ADV: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_timingAdvance;
          #else
            newValue = (int)obd->timingAdvance(); 
          #endif
          break;
    case VIEW_ENGINE_LOAD: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_engineLoad;
          #else
            newValue = (int)obd->engineLoad(); 
          #endif
          break;
    case VIEW_MAF_RATE: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_mafRate;
          #else
            newValue = (int)obd->mafRate(); 
          #endif
          break;
    case VIEW_SHORT_FUEL_TRIM: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_shortFuelTrim;
          #else
            newValue = (int)obd->shortTermFuelTrimBank_1(); 
          #endif
          break;
    case VIEW_LONG_FUEL_TRIM: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_longFuelTrim;
          #else
            newValue = (int)obd->longTermFuelTrimBank_1(); 
          #endif
          break;
    case VIEW_THROTTLE: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_throttle;
          #else
            newValue = (int)obd->throttle(); 
          #endif
          break;
    //supportedPIDs_21_40
    case VIEW_FUEL_LEVEL: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_fuelLevel;
          #else
            newValue = (int)obd->fuelLevel(); 
          #endif
          break;
    //supportedPIDs_41_60
    case VIEW_AMBIENT_TEMP:
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_ambientAirTemp;
          #else
            newValue = (int)obd->ambientAirTemp(); 
          #endif 
          break;
    case VIEW_OIL_TEMP: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_oilTemp;
          #else
            newValue = (int)obd->oilTemp(); 
          #endif
          break;
    case VIEW_ABS_LOAD: 
          #ifdef MOCK_OBD
            newValue = MOCK_OBD_absLoad;
          #else
            newValue = (int)obd->absLoad(); 
          #endif
          break;
    case VIEW_NONE:
          doAction = false;
          debug->println(DEBUG_LEVEL_INFO, "Inactive view");
          break;
    default:
          doAction = false;
          debug->print(DEBUG_LEVEL_ERROR, viewTypeId);
          debug->println(DEBUG_LEVEL_ERROR, " is an unknown view type");
  }

  #ifdef MOCK_OBD
    bool saveValue = true;

    debug->println(DEBUG_LEVEL_DEBUG2, "OBD Read SUCCESS");  
    debug->print(DEBUG_LEVEL_DEBUG2, "----------------> value readed: ");
    debug->println(DEBUG_LEVEL_DEBUG2, newValue);

  #else
    bool saveValue = false;

    if (doAction) {      
      delay(DELAY_READING);
      if (obd->nb_rx_state == ELM_SUCCESS) {
        saveValue = true;
        debug->println(DEBUG_LEVEL_DEBUG, "OBD Read SUCCESS");  
        debug->print(DEBUG_LEVEL_DEBUG, "----------------> value readed: ");
        debug->println(DEBUG_LEVEL_DEBUG, newValue);
      } else if (obd->nb_rx_state != ELM_GETTING_MSG) {
        debug->println(DEBUG_LEVEL_DEBUG, "OBD Read ERROR");  
        newValue = INT_MIN;
        saveValue = true;
      }
    }
  #endif

    if (saveValue) {
      setValueForViewType(viewTypeId, newValue);
    }
  return saveValue;
}

void OdbAdapter::setFoundOBD2(bool found) {
    this->foundOBD2 = found;
}

void OdbAdapter::setDeviceName(String deviceName) {
    this->deviceName = deviceName;
}

void OdbAdapter::setDeviceAddress(String deviceAddr) {
    this->deviceAddr = deviceAddr;
}

//supportedPIDs_1_20
int OdbAdapter::getVoltage() {
    return this->voltage;
}
void OdbAdapter::setVoltage(int voltage) {
    this->voltage = voltage;
}

int OdbAdapter::getKph() {
    return this->kph;
}
void OdbAdapter::setKph(int kph) {
    this->kph = kph;
}

int OdbAdapter::getRpm() {
    return this->rpm;
}
void OdbAdapter::setRpm(int rpm) {
    this->rpm = rpm;
}

int OdbAdapter::getCoolantTemp() {
    return this->coolantTemp;
}
void OdbAdapter::setCoolantTemp(int coolantTemp) {
    this->coolantTemp = coolantTemp;
}

int OdbAdapter::getIntakeTemp() {
    return this->intakeTemp;
}
void OdbAdapter::setIntakeTemp(int intakeTemp) {
    this->intakeTemp = intakeTemp;
}

int OdbAdapter::getTimingAdvance() {
    return this->timingAdvance;
}
void OdbAdapter::setTimingAdvance(int timingAdvance) {
    this->timingAdvance = timingAdvance;
}
int OdbAdapter::getEngineLoad() {
    return this->engineLoad;
}
void OdbAdapter::setEngineLoad(int engineLoad) {
    this->engineLoad = engineLoad;
}

int OdbAdapter::getMafRate() {
    return this->mafRate;
}
void OdbAdapter::setMafRate(int mafRate) {
    this->mafRate = mafRate;
}

int OdbAdapter::getShortFuelTrim() {
    return this->shortFuelTrim;
}
void OdbAdapter::setShortFuelTrim(int shortFuelTrim) {
    this->shortFuelTrim = shortFuelTrim;
}

int OdbAdapter::getLongFuelTrim() {
    return this->longFuelTrim;
}
void OdbAdapter::setLongFuelTrim(int longFuelTrim) {
    this->longFuelTrim = longFuelTrim;
}

int OdbAdapter::getThrottle() {
    return this->throttle;
}
void OdbAdapter::setThrottle(int throttle) {
    this->throttle = throttle;
}

//supportedPIDs_21_40
int OdbAdapter::getFuelLevel() {
    return this->fuelLevel;
}
void OdbAdapter::setFuelLevel(int fuelLevel) {
    this->fuelLevel = fuelLevel;
}

//supportedPIDs_41_60
int OdbAdapter::getAmbientTemp() {
    return this->ambientTemp;
}
void OdbAdapter::setAmbientTemp(int ambientTemp) {
    this->ambientTemp = ambientTemp;
}

int OdbAdapter::getOilTemp() {
    return this->oilTemp;
}
void OdbAdapter::setOilTemp(int oilTemp) {
    this->oilTemp = oilTemp;
}

int OdbAdapter::getAbsLoad() {
    return this->absLoad;
}
void OdbAdapter::setAbsLoad(int absLoad) {
    this->absLoad = absLoad;
}
