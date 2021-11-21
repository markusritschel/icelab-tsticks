#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "DS28EA00.h"

RTC_DS1307 rtc;


int check;
int deviceCount = 0;
float tempC;


void setup() {
  Serial.begin(9600);
  // initialize rtc
  if (!rtc.begin()) {
    Serial.println(F("# Couldn't find RTC"));
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println(F("# RTC is NOT running!"));
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
