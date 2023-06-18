#ifndef BLE_H_
#define BLE_H_

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "BluefruitConfig.h"
#include "Logger.h"
#include "config.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

class Ble {
public:
    struct BleData {
      int serviceId;
      int reqSpeed;
      int reqState;
      int reqTorque;
      int reqAccel;
      int reqRegen;
      int resMotorTemp;
      int resInvTemp;
      int resTorque;
      int resSpeed;
      int resState;
      int resDcVolt;
      int resDcCurrent;
      bool outMainCon;
      bool outPreCon;
      bool outBrake;
      bool outCooling;
      bool outReverseLight;
      bool inReverse;
      bool inEnable;
      int inThrottle;
      int inBrake;
      bool isRunning;
      bool isFaulted;
      bool isWarning;
      
      int configSpeedMax;
      int configTorqueMax;
      int configSpeedSlewRate;
      int configTorqueSlewRate;
      int configReversePercent;
      int configKilowattHrs;
      int configPrechargeR;
      int configNominalVolt;
      int configPrechargeRelay;
      int configMainContactorRelay;
      int configCoolFan;
      int configCoolOn;
      int configCoolOff;
      int configBrakeLight;
      int configRevLight;
      int configEnableIn;
      int configReverseIn;
      int configRegenTaperLower;
      int configRegenTaperUpper;
    };
    void setup();
    void updateValues(BleData *data);
private:
    void sendValue(int value, int id);
    void sendValue(bool value, int id);
};

#endif /* BLE_H_ */

