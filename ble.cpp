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

  // DMOC REQUESTED
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A1B, PROPERTIES=0x10, MIN_LEN=1, MAX_LEN=5, VALUE=0"), &tVoltId);
  // 0x232
  // speed 

  // state

  //0x233
  // torque

  //0x234
  // accelerate
  
  //regen

  // 0x236
  // shifter position

  // DMOC RECEIVED
  // temprature status 0x651
  // motor temp

  // inv temp


  //torque report 0x23A
  // actual torque

  // speed and current 0x23B
  // speed

  // state

  // HV bus state 0x650
  // dc voltage

  // dc current
 


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
