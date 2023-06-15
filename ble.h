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
    void setup();
    void updateValues(BleData data);
     struct BleData {
      int serviceId;
      int reqSpeed;
      int reqState;
      int reqTorque;
      int reqAccel;
      int reqRegen;
      int reqShifter;
      int resMotorTemp;
      int resInvTemp;
      int resToruq;
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
      int inThrottle2;
      int inBrake;
};
private:
    void Ble::sendValue(int value, int id);
};

#endif /* BLE_H_ */

