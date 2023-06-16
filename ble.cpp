#include "ble.h"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
int32_t serviceId;
int32_t reqSpeed;
int32_t reqState;
int32_t reqTorque;
int32_t reqAccel;
int32_t reqRegen;
int32_t resMotorTemp;
int32_t resInvTemp;
int32_t resTorque;
int32_t resSpeed;
int32_t resState;
int32_t resDcVolt;
int32_t resDcCurrent;
int32_t outMainCon;
int32_t outPreCon;
int32_t outBrake;
int32_t outCooling;
int32_t outReverseLight;
int32_t inReverse;
int32_t inEnable;
int32_t inThrottle;
int32_t inBrake;
int32_t isRunning;
int32_t isFaulted;
int32_t isWarning;

void Ble::setup() {

  if ( !ble.begin(VERBOSE_MODE) )
  {
    Logger::debug("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?");
  }

  /* Perform a factory reset to make sure everything is in a known state */
  Logger::debug("Performing a factory reset: ");
  if (! ble.factoryReset() ){
       Logger::debug("Couldn't factory reset");
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Logger::debug("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Change the device name to make it easier to find */
  Logger::debug("Setting device name to %s': pao EVCU");

  if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Pao EVCU")) ) {
    Logger::debug("Could not set device name?");
  }

  /* Add the Service definition */
  /* Service ID should be 1 */
  Logger::debug("Adding the Service definition (UUID = 0x27B1): ");
  bool success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x180F"), &serviceId);
  if (! success) {
    Logger::debug("Could not add service");
  }

  // DMOC REQUESTED
  // 0x232
  // speed 
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF01, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &reqSpeed);

  // state
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF02, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &reqState);

  //0x233
  // torque
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF03, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &reqTorque);

  //0x234
  // accelerate
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF04, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &reqAccel);
  
  //regen
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF05, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &reqRegen);

  // DMOC RECEIVED
  // temprature status 0x651
  // motor temp
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF07, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &resMotorTemp);

  // inv temp
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF08, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &resInvTemp);


  //torque report 0x23A
  // actual torque
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF09, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &resTorque);

  // speed and current 0x23B
  // speed
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF0A, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &resSpeed);

  // state
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF0B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &resState);

  // HV bus state 0x650
  // dc voltage
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF0C, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &resDcVolt);

  // dc current
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF0D, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &resDcCurrent);

  // OUTPUTS
  // main contactor
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF0E, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=1, VALUE=0"), &outMainCon);

  //precharge contactor
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF0F, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=1, VALUE=0"), &outPreCon);

  //brake
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF10, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=1, VALUE=0"), &outBrake);

  //cooling
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF11, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=1, VALUE=0"), &outCooling);

  //reverse light
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF12, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=1, VALUE=0"), &outReverseLight);


  // INPUTS

  // reverse
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF13, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=1, VALUE=0"), &inReverse);

  //enable
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF14, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=1, VALUE=0"), &inEnable);

  //throttle 1
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF15, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &inThrottle);


  // brake 
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF17, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &inBrake);

  // STATUS
  // running
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF18, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=1, VALUE=0"), &isRunning);
  // fault
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF19, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=1, VALUE=0"), &isFaulted);
  // warning
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0xFF1A, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=1, VALUE=0"), &isWarning);
 


  ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-05-02-0d-18-0a-18") );

  /* Reset the device for the new service setting changes to take effect */
  Serial.print(F("Performing a SW reset (service changes require a reset): "));
  ble.reset();
}

void Ble::sendValue(int value, int id) {
  ble.print( F("AT+GATTCHAR=") );
  ble.print( id );
  ble.print( F(",") );
  ble.println(value, HEX);

}

void Ble::sendValue(bool value, int id) {
  ble.print( F("AT+GATTCHAR=") );
  ble.print( id );
  ble.print( F(",") );
  ble.println(value, HEX);

}

void Ble::updateValues(BleData *data) {
  Ble::sendValue(data->reqSpeed, reqSpeed);
  Ble::sendValue(data->reqState, reqState);
  Ble::sendValue(data->reqTorque, reqTorque);
  Ble::sendValue(data->reqAccel, reqAccel);
  Ble::sendValue(data->reqRegen, reqRegen);
  Ble::sendValue(data->resMotorTemp, resMotorTemp);
  Ble::sendValue(data->resInvTemp, resInvTemp);
  Ble::sendValue(data->resTorque, resTorque);
  Ble::sendValue(data->resSpeed, resSpeed);
  Ble::sendValue(data->resState, resState);
  Ble::sendValue(data->resDcVolt, resDcVolt);
  Ble::sendValue(data->resDcCurrent, resDcCurrent);
  Ble::sendValue(data->outMainCon, outMainCon);
  Ble::sendValue(data->outPreCon, outPreCon);
  Ble::sendValue(data->outBrake, outBrake);
  Ble::sendValue(data->outCooling, outCooling);
  Ble::sendValue(data->outReverseLight, outReverseLight);
  Ble::sendValue(data->inReverse, inReverse);
  Ble::sendValue(data->inEnable, inEnable);
  Ble::sendValue(data->inThrottle, inThrottle);
  Ble::sendValue(data->inBrake, inBrake);
  Ble::sendValue(data->isRunning, isRunning);
  Ble::sendValue(data->isFaulted, isFaulted);
  Ble::sendValue(data->isWarning, isWarning);
}
