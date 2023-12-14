#ifndef DATA_OBD_h
#define DATA_OBD_h

#include <Arduino.h>

#include "defines.h"

class BluetoothOBD {
  private:
    bool btConnected;
    bool obdConnected;

    int voltage;
    int kph;
    int rpm;
    int coolantTemp;
    int ambientTemp;
    int intakeTemp;
    int timingAdvance;

    String obdDeviceName;
    String obdDeviceAddr;

    esp_spp_sec_t sec_mask;
    esp_spp_role_t role;

    esp_bd_addr_t client_addr = {0x00,0x00,0x00,0x00,0x00,0x00};
    String deviceName;
    String deviceAddr;

    String ByteArraytoString(esp_bd_addr_t bt_address);
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

    int getVoltage();
    int getKph();
    int getRpm();
    int getCoolantTemp();
    int getAmbientTemp();
    int getIntakeTemp();
    int getTimingAdvance();

    void setVoltage(int voltage);
    void setKph(int kph);
    void setRpm(int rpm);
    void setCoolantTemp(int coolantTemp);
    void setAmbientTemp(int ambientTemp);
    void setIntakeTemp(int intakeTemp);
    void setTimingAdvance(int timingAdvance);
    
};

#endif