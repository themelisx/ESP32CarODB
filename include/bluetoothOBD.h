#ifndef DATA_OBD_h
#define DATA_OBD_h

#include <Arduino.h>

#include "defines.h"

class BluetoothOBD {
  private:
    bool btConnected;
    bool obdConnected;

    String obdDeviceName;
    String obdDeviceAddr;

    esp_spp_sec_t sec_mask;
    esp_spp_role_t role;

    esp_bd_addr_t client_addr = {0x00,0x00,0x00,0x00,0x00,0x00};
    String deviceName;
    String deviceAddr;

    String ByteArraytoString(esp_bd_addr_t bt_address);

    void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

    //supportedPIDs_1_20
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

    //supportedPIDs_21_40
    int fuelLevel;

    //supportedPIDs_41_60
    int ambientTemp;
    int oilTemp;
    int absLoad;

    bool scanBTdevice();

  public:
    BluetoothOBD(String obdDeviceName, String obdDeviceAddr);
    bool connect(char *pin);
    void disconnect();
    void setBtConnected(bool connected);
    void setObdConnected(bool connected);
    bool isBluetoothConnected();
    bool isOBDConnected();

    void setDeviceName(String deviceName);
    void setDeviceAddress(String deviceAddr);

    //supportedPIDs_1_20
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

    //supportedPIDs_21_40
    int getFuelLevel();
    void setFuelLevel(int fuelLevel);

    //supportedPIDs_41_60
    int getAmbientTemp();
    void setAmbientTemp(int ambientTemp);

    int getOilTemp();
    void setOilTemp(int oilTemp);

    int getAbsLoad();
    void setAbsLoad(int absLoad);
    
};

#endif