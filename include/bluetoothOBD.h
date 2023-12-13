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

    String ByteArraytoString(esp_bd_addr_t bt_address);
    bool scanBTdevice();

  public:
    BluetoothOBD();
    bool connect(char *pin);
    void disconnect();
    void setBtConnected(bool connected);
    void setObdConnected(bool connected);
    bool isBluetoothConnected();
    bool isOBDConnected();

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