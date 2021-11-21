#include <SD.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "DS28EA00.h"

RTC_DS1307 rtc;


int check;
int deviceCount = 0;
float tempC;


void setup() {
  const int chipSelect = 10;

  Serial.begin(9600);
  // initialize rtc
  if (!rtc.begin()) {
    Serial.println(F("# Couldn't find RTC"));
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println(F("# RTC is NOT running!"));
  }

  // initialize SD card
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println(F("# Card failed, or not present"));
    return;     // don't do anything more:
  } else {
    Serial.println(F("# STATUS: card initialized"));
  }

}


void loop() {

  // iterate over all possible pins
  for (uint8_t pin=0; pin < 15; pin++) {

    // check if pin is occupied by a DS28EA00 device; skip if not
    check = check_pin_for_device(pin);
    if (check == -1) {
      continue;
    }

    if (pin <= 9) {
      Serial.print(0);
    }
    Serial.print(pin);
    Serial.print(":\t");
    
    OneWire  oneWire(pin);
    DallasTemperature sensors(&oneWire);
    sensors.begin();  // Start up the library
    deviceCount = sensors.getDeviceCount();
    
    // Send command to all the sensors for temperature conversion
    sensors.requestTemperatures(); 

    tstick_t tstick = init_tstick(pin);

    // Display temperature from each sensor
    for (int i = 0;  i < deviceCount;  i++)
    {
      tempC = sensors.getTempCByIndex(i);
      Serial.print(tempC);
      Serial.print("\t");
    }
    Serial.println();
  }
  delay(2000);
}


/**********************************************************************
* Function: init_tstick
* Parameters: pin
* Returns: tstick_t
*
* Description: Initialize a T-Stick
**********************************************************************/
tstick_t init_tstick(uint8_t pin) {
  // init one-wire bus and sensors
  OneWire ow_bus(pin);
  DallasTemperature sensors(&ow_bus);

  // create tstick object and populate
  tstick_t tstick;
  tstick.pin = pin;
  tstick.ow_bus = ow_bus;
  tstick.sensors = sensors;

  uint8_t numSensors = sensors.getDeviceCount();
    
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
  // initialize OneWire bus
  OneWire ow_bus(pin);
  byte addr[8] = {0,0,0,0,0,0,0,0};
  
  // Serial.print("Checking PIN ");
  // Serial.print(pin);
  // Serial.print(": ");

  // check for sensors by searching OneWire bus
  if ( !ow_bus.search(addr) ) {
    // Serial.println("No sensors detected...");
  	return(-1);
  }
  // check if devices are DS28EA00 sensors
  else if ( addr[0] != 0x42 ) {
  	// Serial.println("Not a DS28EA00 sensor");
  	return(-1);
  }
  
  // otherwise: reset_search and return 0
  // Serial.println("DS28EA00 device found");
  ow_bus.reset_search();
  return(0);
}
/********** END check_pin_for_device *********************************/
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
int ds28ea00_sequence_discoverey(OneWire ow_bus, ds28ea00_t *device_array)
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

    // retrieve 64-bit Registration Number (8 byte)
    test_end_of_bus = 0xFF;
    for(idy = 0; idy < 8; idy++)
    { 
      data = ow_bus.read();
      test_end_of_bus &= data;
      device_array[idx].rom_code[idy] = data;
    }

    // Test for End of bus; No response: all devices have been discovered
    if(test_end_of_bus == 0xFF)
    {
      break;
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
  while(test_end_of_bus != 0xFF);
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
