#include <Arduino.h>
#include "defines.h"

#ifdef USE_OBD_BLUETOOTH
    #include <BluetoothSerial.h>
#endif

#include <ELMduino.h>

#include "odbAdapter.h"
#include "gauge.h"

#include "debug.h"
#include "vars.h"

OdbAdapter::OdbAdapter(String deviceName, String deviceAddr) {

    debug->println(DEBUG_LEVEL_DEBUG, "[OdbAdapter]");

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

    debug->println(DEBUG_LEVEL_DEBUG, "[OK]");
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
  return deviceConnected;
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

void OdbAdapter::updateOBDValue(Gauge* gauge) {
    //Gauge* gauge = displayManager->getDisplay(displayManager->getActiveDisplayId())->getActiveGauge();

    if (isDeviceConnected() && isOBDConnected()) {
      debug->println(DEBUG_LEVEL_DEBUG, "Reading OBD...");

      bool valueReaded = readValueForViewType(gauge->getId());

      int newValue = INT_MIN;
      if (valueReaded) {
        
        newValue = getValueForViewType(gauge->getId());

        if (gauge->data.value != newValue) {
          debug->println(DEBUG_LEVEL_DEBUG2, "Value has changed");
          /*
          debug->print(DEBUG_LEVEL_DEBUG2, "---> new value : ");
          debug->println(DEBUG_LEVEL_DEBUG2, newValue);
          debug->print(DEBUG_LEVEL_DEBUG2, "---> old value : ");
          debug->println(DEBUG_LEVEL_DEBUG2, gauge->data.value);
          */
        }
        gauge->data.value = newValue;

        int secondaryViewIdx = gauge->secondaryViews.activeViewIndex;
        if (secondaryViewIdx != 0) {
          int secondaryViewId = gauge->secondaryViews.ids[secondaryViewIdx];

          valueReaded = readValueForViewType(secondaryViewId);
          if (valueReaded) {
            newValue = getValueForViewType(secondaryViewId);
            if (gauge->secondaryViews.oldValue[secondaryViewIdx] != newValue) {
              debug->println(DEBUG_LEVEL_DEBUG2, "Secondary value has changed");
            }
            gauge->secondaryViews.value[secondaryViewIdx] = newValue;
          }
        }
      } else {
          debug->println(DEBUG_LEVEL_DEBUG2, "value NOT readed");
      }
    }
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
    case VIEW_FUEL_LEVEL: newValue = getFuelLevel(); break;
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
    case VIEW_FUEL_LEVEL: setFuelLevel(newValue); break;
    case VIEW_AMBIENT_TEMP: setAmbientTemp(newValue); break;
    case VIEW_OIL_TEMP: setOilTemp(newValue); break;
    case VIEW_ABS_LOAD: setAbsLoad(newValue); break;
    case VIEW_NONE: break;    
    default: break;
  }
}

#ifdef MOCK_OBD
int OdbAdapter::getRandomNumber(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}
#endif

bool OdbAdapter::readObdValue(int viewTypeId) {

  int newValue = 0;

  #ifdef MOCK_OBD
    bool saveValue = true;

    switch (viewTypeId) {
      case VIEW_BATTERY_VOLTAGE: 
              newValue = getRandomNumber(MOCK_OBD_batteryVoltage - 20, MOCK_OBD_batteryVoltage + 20); break;
      case VIEW_KPH: 
              newValue = getRandomNumber(MOCK_OBD_kph - 20, MOCK_OBD_kph + 20); break;
      case VIEW_RPM: 
              newValue = getRandomNumber(MOCK_OBD_rpm - 1000, MOCK_OBD_rpm + 1000); break;
      case VIEW_COOLANT_TEMP: 
              newValue = getRandomNumber(MOCK_OBD_engineCoolantTemp - 30, MOCK_OBD_engineCoolantTemp + 30); break;    
      case VIEW_INTAKE_TEMP:
              newValue = getRandomNumber(MOCK_OBD_intakeAirTemp - 20, MOCK_OBD_intakeAirTemp + 20); break;
      case VIEW_TIMING_ADV: 
              newValue = getRandomNumber(MOCK_OBD_timingAdvance - 5, MOCK_OBD_timingAdvance + 5); break;
      case VIEW_ENGINE_LOAD: 
              newValue = getRandomNumber(MOCK_OBD_engineLoad - 10, MOCK_OBD_engineLoad + 10); break;
      case VIEW_MAF_RATE: 
              newValue = getRandomNumber(MOCK_OBD_mafRate - 3, MOCK_OBD_mafRate + 3); break;
      case VIEW_SHORT_FUEL_TRIM: 
              newValue = getRandomNumber(MOCK_OBD_shortFuelTrim - 5, MOCK_OBD_shortFuelTrim + 5); break;
      case VIEW_LONG_FUEL_TRIM: 
              newValue = getRandomNumber(MOCK_OBD_longFuelTrim - 5, MOCK_OBD_longFuelTrim + 5); break;
      case VIEW_THROTTLE: 
              newValue = getRandomNumber(MOCK_OBD_throttle - 10, MOCK_OBD_throttle + 10); break;
      case VIEW_FUEL_LEVEL: 
              newValue = getRandomNumber(MOCK_OBD_fuelLevel - 15, MOCK_OBD_fuelLevel + 15); break;
      case VIEW_AMBIENT_TEMP:
              newValue = getRandomNumber(MOCK_OBD_ambientAirTemp - 10, MOCK_OBD_ambientAirTemp + 10); break;
      case VIEW_OIL_TEMP: 
              newValue = getRandomNumber(MOCK_OBD_oilTemp - 15, MOCK_OBD_oilTemp + 15); break;
      case VIEW_ABS_LOAD: 
              newValue = getRandomNumber(MOCK_OBD_absLoad - 5, MOCK_OBD_absLoad + 5); break;
      case VIEW_NONE:
            debug->println(DEBUG_LEVEL_INFO, "Inactive view");
            break;
      default:
            debug->print(DEBUG_LEVEL_ERROR, viewTypeId);
            debug->println(DEBUG_LEVEL_ERROR, " is an unknown view type");
    }

    debug->println(DEBUG_LEVEL_DEBUG2, "OBD Read SUCCESS");  
    debug->print(DEBUG_LEVEL_DEBUG2, "----------------> value readed: ");
    debug->println(DEBUG_LEVEL_DEBUG2, newValue);

  #else
    bool saveValue = false;
    bool doAction = true;

    switch (viewTypeId) {
      case VIEW_BATTERY_VOLTAGE: 
            newValue = (int)(obd->batteryVoltage() * 10.0); break;
      case VIEW_KPH:
            newValue = (int)obd->kph(); break;
      case VIEW_RPM: 
            newValue = (int)obd->rpm(); break;
      case VIEW_COOLANT_TEMP: 
            newValue = (int)obd->engineCoolantTemp(); break;    
      case VIEW_INTAKE_TEMP:
            newValue = (int)obd->intakeAirTemp(); break;
      case VIEW_TIMING_ADV: 
            newValue = (int)obd->timingAdvance(); break;
      case VIEW_ENGINE_LOAD: 
            newValue = (int)obd->engineLoad(); break;
      case VIEW_MAF_RATE: 
            newValue = (int)obd->mafRate(); break;
      case VIEW_SHORT_FUEL_TRIM: 
            newValue = (int)obd->shortTermFuelTrimBank_1(); break;
      case VIEW_LONG_FUEL_TRIM: 
            newValue = (int)obd->longTermFuelTrimBank_1(); break;
      case VIEW_THROTTLE: 
            newValue = (int)obd->throttle(); break;
      case VIEW_FUEL_LEVEL: 
            newValue = (int)obd->fuelLevel(); break;
      case VIEW_AMBIENT_TEMP:
            newValue = (int)obd->ambientAirTemp(); break;
      case VIEW_OIL_TEMP: 
            newValue = (int)obd->oilTemp(); break;
      case VIEW_ABS_LOAD: 
            newValue = (int)obd->absLoad(); break;
      case VIEW_NONE:
            doAction = false;
            debug->println(DEBUG_LEVEL_INFO, "Inactive view");
            break;
      default:
            doAction = false;
            debug->print(DEBUG_LEVEL_ERROR, viewTypeId);
            debug->println(DEBUG_LEVEL_ERROR, " is an unknown view type");
    }

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

int OdbAdapter::getFuelLevel() {
    return this->fuelLevel;
}
void OdbAdapter::setFuelLevel(int fuelLevel) {
    this->fuelLevel = fuelLevel;
}

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
