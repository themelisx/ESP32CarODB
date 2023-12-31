#ifndef DATA_OBD_h
#define DATA_OBD_h

#include <Arduino.h>

#include "defines.h"

#ifdef USE_OBD_BLUETOOTH
    #include <BluetoothSerial.h>
#endif

#ifdef USE_OBD_WIFI
  #ifdef ESP32
    #include <WiFi.h>
  #endif
  #ifdef ESP8266
    #include <ESP8266WiFi.h>
  #endif
#endif

class OdbAdapter {
  private:

    #ifdef USE_OBD_BLUETOOTH
      BluetoothSerial SerialDevice;
      esp_spp_sec_t sec_mask;
      esp_spp_role_t role;
      esp_bd_addr_t client_addr = {0x00,0x00,0x00,0x00,0x00,0x00};
      
      String ByteArraytoString(esp_bd_addr_t bt_address);
      //void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
    #endif

    #ifdef USE_OBD_WIFI
      WiFiClient SerialDevice;
      const char* ssid = "WiFi_OBDII";
      const char* password = "your-password";
      #ifdef ESP32
        IPAddress server(192, 168, 0, 10);
      #endif
      #ifdef ESP8266
        const char* server = "192.168.0.10";
      #endif
    #endif

    bool deviceConnected;
    bool obdConnected;
    
    String deviceName;
    String deviceAddr;
    bool foundOBD2;

    int voltage;
    int kph;
    int rpm;
    int coolantTemp;
    int intakeTemp;
    int timingAdvance; 
    int engineLoad;
    int mafRate;
    int shortFuelTrim;
    int longFuelTrim;
    int throttle;

    int fuelLevel;

    int ambientTemp;
    int oilTemp;
    int absLoad;

    bool scanBTdevice();
    int getRandomNumber(int min, int max);

  public:
    OdbAdapter(String deviceName, String deviceAddr);

    void setFoundOBD2(bool found);

    bool connect(char *pin);
    void disconnect();
    void setDeviceConnected(bool connected);
    void setObdConnected(bool connected);
    bool isDeviceConnected();
    bool isOBDConnected();

    void setDeviceName(String deviceName);
    void setDeviceAddress(String deviceAddr);

    bool readValueForViewType(int viewId);
    int getValueForViewType(int viewId);
    void setValueForViewType(int viewTypeId, int newValue);
    bool readObdValue(int viewTypeId);

    int getVoltage();
    void setVoltage(int voltage);

    int getKph();
    void setKph(int kph);

    int getRpm();
    void setRpm(int rpm);

    int getCoolantTemp();
    void setCoolantTemp(int coolantTemp);

    int getIntakeTemp();
    void setIntakeTemp(int intakeTemp);

    int getTimingAdvance();
    void setTimingAdvance(int timingAdvance);

    int getEngineLoad();
    void setEngineLoad(int engineLoad);

    int getMafRate();
    void setMafRate(int mafReate);

    int getShortFuelTrim();
    void setShortFuelTrim(int shortFuelTrim);

    int getLongFuelTrim();
    void setLongFuelTrim(int longFuelTrim);

    int getThrottle();
    void setThrottle(int throttle);

    int getFuelLevel();
    void setFuelLevel(int fuelLevel);

    int getAmbientTemp();
    void setAmbientTemp(int ambientTemp);

    int getOilTemp();
    void setOilTemp(int oilTemp);

    int getAbsLoad();
    void setAbsLoad(int absLoad);
    
};

#endif