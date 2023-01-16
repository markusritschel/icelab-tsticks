/*************************************************************************************************
 * Sketch:  icelab-tsticks.ino
 * Author:  M. Ritschel
 * E-Mail:  kontakt@markusritschel.de
 * Version: 2022-01-16
 *
 * This script empowers an Arduino mikrocontroller to read-out the temperature sticks (T-Sticks)
 * that have been constructed by Leif Riemenschneider. These T-Sticks consist of an array of 
 * DS28EA00 1-wire sensors and can be read-out in their physical order.
 * 
 ************************************************************************************************/
#include <SD.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "DS28EA00.h"

RTC_DS1307 rtc;

// uncomment the following line for debugging:
//#define DEBUG 1
#ifdef DEBUG
  #define DEBUG_PRINT(txt) Serial.print(txt);
  #define DEBUG_PRINTLN(txt) Serial.println(txt);
#else
  #define DEBUG_PRINT(txt)
  #define DEBUG_PRINTLN(txt)
#endif

int check;
float tempC;


/**********************************************************************
* SETUP LOOP
* Initializes modules (RTC, SD-Card) etc.
**********************************************************************/
void setup() {

  while (!Serial);
  Serial.begin(9600);

  DEBUG_PRINTLN(F(" > Debugger ON"));

  // initialize rtc
  if (!rtc.begin()) {
    Serial.println(F("# Couldn't find RTC"));
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println(F("# RTC is NOT running!"));
  }
  // The following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // - TimeSpan(0,1,59,50));   // adjust clock via local time to UTC
  // Serial.println(F("RTC adjusted to UTC time!"));
  // It must be set initially at least once, and then only if the backup battery doesn't supply 
  // the RTC anymore (e.g. if the Arduino is w/o power for longer time) or if you have to change the time/data information

  // initialize SD card
  // see if the card is present and can be initialized:
  if (!SD.begin(10)) {
    Serial.println(F("# Card failed, or not present"));
    return;     // don't do anything more:
  } else {
    DEBUG_PRINTLN(F(" > STATUS: card initialized"));
  }

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  
  // ========== write header to SD card ==========
  writeln2SD("");
  writeln2SD(F("# =========================================== "));
  write2SD(F("# Initialization time: "));
  writeln2SD(getISOtime());
  writeln2SD(F("# <module>, <HEX ID>, <timestamp>, <sensors>..."));
  writeln2SD(F("# ------------------------------------------- "));
  
}
/********** END SETUP LOOP *******************************************/


/**********************************************************************
  MAIN LOOP
  Holds the main routines to read the t-sticks found on different ports
 - Check PINs on Arduino for DS28EA00 sensors
 - Initializes T-Stick and discovers the physical sequence on the bus
 - Read temperatures for every sensor on the bus
 - Write out the values to Serial Monitor and to SD card
**********************************************************************/
void loop() {
  // turn on LED for reading status
  digitalWrite(LED_BUILTIN, HIGH);

  // iterate over all possible pins
  for (uint8_t pin=0; pin < 10; pin++) {
    // check if pin is occupied by a DS28EA00 device; skip if not
    check = check_pin_for_device(pin);
    if (check == -1) {
      continue;
    }

    DEBUG_PRINT(F(" > Initializing T-Stick on pin "));
    DEBUG_PRINTLN(pin);
    tstick_t tstick = init_tstick(pin);

    // Display temperature from each sensor
    DEBUG_PRINT(F(" > Requesting temperatures from all devices on the bus on pin "));
    DEBUG_PRINTLN(pin);
    tstick.sensors.requestTemperatures();

    write2SD(String(twodigits(pin)));
    write2SD(csv_sep);
    write2SD(tstick.registration_number);
    write2SD(csv_sep);
    write2SD(getISOtime());
    
    for(uint8_t i = 0; i < 8; i++)
    {
      tempC = tstick.sensors.getTempC(tstick.sensor_array[i].rom_code);
      write2SD(csv_sep);
      write2SD(String(tempC));
    }

    writeln2SD("");
  }
  digitalWrite(LED_BUILTIN, LOW);
  delay(10000);
}
/********** END MAIN LOOP ********************************************/


/**********************************************************************
* Function: init_tstick
* Parameters: pin
* Returns: tstick_t
*
* Description: Initialize a T-Stick
**********************************************************************/
tstick_t init_tstick(uint8_t pin) {
  OneWire ow_bus(pin);
  DallasTemperature sensors(&ow_bus);

  tstick_t tstick;
  tstick.pin = pin;
  tstick.ow_bus = ow_bus;
  tstick.sensors = sensors;
  detect_ds28ea00_devices(&tstick);

  return(tstick);
}
/********** END init_tstick ******************************************/


/**********************************************************************
* Function: check_pin_for_device
* Parameters: pin
* Returns: int
*
* Description: Check if a pin is occupied by a T-Stick
**********************************************************************/
int check_pin_for_device(uint8_t pin)
{
  OneWire ow_bus(pin);
  byte addr[8] = {0,0,0,0,0,0,0,0};
  
  if ( !ow_bus.search(addr) ) {
    DEBUG_PRINT(F(" > No sensors detected on pin "));
    DEBUG_PRINTLN(pin);
  	return(-1);
  }
  else if ( addr[0] != IS_DS28EA00_SENSOR ) {
  	DEBUG_PRINT(F(" > Not a DS28EA00 sensor on pin "));
  	DEBUG_PRINTLN(pin);
  	return(-1);
  }
  else {
    DEBUG_PRINT(F(" > DS28EA00 device found on pin "));
    DEBUG_PRINTLN(pin);
    ow_bus.reset_search();
    return(0);
  }
}
/********** END check_pin_for_device *********************************/


/**********************************************************************
* Function: detect_ds28ea00_devices
* Parameters: ds28ea00_t device_array - Pointer to an array of DS28EA00 sensors
* Returns: void 
*
* Description: Detect and sort DS28EA00 sensors in device_array
**********************************************************************/
void detect_ds28ea00_devices(tstick_t *tstick)
{
  int state = 0;
  OneWire bus = tstick->ow_bus;
  ds28ea00_t sensor_array[10];
  
  do
  {
    DEBUG_PRINTLN(F(" > Detecting physical sequence of sensors on bus..."));
    state = ds28ea00_sequence_discoverey(bus, sensor_array, tstick);
    if(state == -1)
    {
      Serial.println("Error!");
    }
  }
  while(state == -1);

  DEBUG_PRINTLN(F(" > Sequence detected. Populating sensor array with addresses."));
  for (uint8_t i=0; i < 8; i++) {
    tstick->sensor_array[i] = sensor_array[i];
  }

  return;
  
}
/********** END detect_ds28ea00_devices ******************************/


/**********************************************************************
* Function: ds28ea00_sequence_discoverey
* Parameters: OneWire *ow_bus - Pointer to the OneWire bus
*             ds28ea00_t *device_array - Pointer to an array of type 
*                                        ds28ea00_t
* Returns: none
*
* Description: Detects devices on the bus and populates the 64-bit rom
*              codes of the device_array
**********************************************************************/
int ds28ea00_sequence_discoverey(OneWire ow_bus, ds28ea00_t *device_array, tstick_t *tstick)
{
  unsigned char test_end_of_bus;
  unsigned char data;
  unsigned char idx = 0;
  unsigned char idy = 0;
  unsigned char num_ds28ea00_on_bus = 0;

  // ---- Start 1st sequence ---- 
  ow_bus.reset();
  ow_bus.skip();  
  ow_bus.write(CHAIN);
  ow_bus.write(CHAIN_ON);
  ow_bus.write(~CHAIN_ON);
  
  data = ow_bus.read();
  if(data != VALID_SEQUENCE)
  {
    return(-1);
  }
  // ---- END 1st sequence ---- 
  
  // ---- Start 2nd sequence ---- 
  do
  {
    ow_bus.reset();
    ow_bus.write(CONDITIONAL_READ_ROM);

    // retrieve 64-bit Registration Number (8 byte) aka ROM code
    test_end_of_bus = END_OF_BUS;
    for(idy = 0; idy < 8; idy++)
    { 
      data = ow_bus.read();
      test_end_of_bus &= data;
      device_array[idx].rom_code[idy] = data;
    }

    // Test for End of bus; No response: all devices have been discovered
    if(test_end_of_bus == END_OF_BUS)
    {
      break;
    }

    // record the first 24 bits of the serial number (as HEX) of the first sensor as identifier for the T-Stick
    if (idx == 0) {
      for(idy = 1; idy <= 3; idy++)
      { 
        tstick->registration_number += String(device_array[idx].rom_code[idy], HEX);
      }
    }

    // If not end of bus: a new device has been detected
    idx++;
    num_ds28ea00_on_bus = idx;
    ow_bus.reset();
    ow_bus.write(PIO_ACCESS_WRITE);   // TODO: Not MATCH_ROM ?
    ow_bus.write(CHAIN);
    ow_bus.write(CHAIN_DONE);
    ow_bus.write(~CHAIN_DONE);
  
    data = ow_bus.read();
    if(data != VALID_SEQUENCE)
    {
      return(-1);
    }
  }
  while(test_end_of_bus != END_OF_BUS);
  // ---- END 2nd sequence ---- 
  
  // ---- Start last sequence ---- 
  ow_bus.reset();
  ow_bus.skip();
  ow_bus.write(CHAIN);
  ow_bus.write(CHAIN_OFF);
  ow_bus.write(~CHAIN_OFF);
  
  data = ow_bus.read();
  if(data != VALID_SEQUENCE)
  {
    return(-1);
  }
  // ---- END last sequence ---- 

  return(0);
}

/********** END ds28ea00_sequence_discoverey**************************/
