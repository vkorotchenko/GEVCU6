#include "ble.h"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
int32_t serviceId;
int32_t tVoltId;
int32_t tAmpId;
int32_t cVoltId;
int32_t cAmpId;
int32_t rTime;

void Ble::setup() {

  if ( !ble.begin(VERBOSE_MODE) )
  {
    Logger::log("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?");
  }

  /* Perform a factory reset to make sure everything is in a known state */
  Logger::log("Performing a factory reset: ");
  if (! ble.factoryReset() ){
       Logger::log("Couldn't factory reset");
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Logger::log("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Change the device name to make it easier to find */
  Logger::log("Setting device name to %s': ", DISPLAY_NAME);

  if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Pao EVCU")) ) {
    Logger::log("Could not set device name?");
  }

  /* Add the Service definition */
  /* Service ID should be 1 */
  Logger::log("Adding the Service definition (UUID = 0x27B0): ");
  bool success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x180F"), &serviceId);
  if (! success) {
    Logger::log("Could not add service");
  }

  // DMOC REQUESTED // TODO update uuids and len
  // 0x232
  // speed 
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  // state
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  //0x233
  // torque
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  //0x234
  // accelerate
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);
  
  //regen
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  // 0x236
  // shifter position
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  // DMOC RECEIVED
  // temprature status 0x651
  // motor temp
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  // inv temp
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);


  //torque report 0x23A
  // actual torque
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  // speed and current 0x23B
  // speed
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  // state
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  // HV bus state 0x650
  // dc voltage
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  // dc current
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  // OUTPUTS
  // main contactor
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  //precharge contactor
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  //brake
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  //cooling
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  //reverse light
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);


  // INPUTS

  // reverse
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  //enable
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  //throttle 1
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  // throttle 2
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);

  // brake 
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);
 


  ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-05-02-0d-18-0a-18") );

  /* Reset the device for the new service setting changes to take effect */
  Serial.print(F("Performing a SW reset (service changes require a reset): "));
  ble.reset();
}

void Ble::updateValues() {
  ble.print( F("AT+GATTCHAR=") );
  ble.print( tVoltId );
  ble.print( F(",") );
  ble.println(tVolt, HEX);
  
  ble.print( F("AT+GATTCHAR=") );
  ble.print( tAmpId );
  ble.print( F(",") );
  ble.println(tAmp, HEX);
  
  ble.print( F("AT+GATTCHAR=") );
  ble.print( cVoltId );
  ble.print( F(",") );
  ble.println(cVolt, HEX);
  
  ble.print( F("AT+GATTCHAR=") );
  ble.print( cAmpId );
  ble.print( F(",") );
  ble.println(cAmp, HEX);
  
  ble.print( F("AT+GATTCHAR=") );
  ble.print( rTime );
  ble.print( F(",") );
  ble.println(running_time, HEX);
}
