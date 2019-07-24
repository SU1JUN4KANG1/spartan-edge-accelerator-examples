/*
  led-and-button

  use button USER1 to control LED1/2 reverse

  The MIT License (MIT)
  Copyright (C) 2019  Seeed Technology Co.,Ltd.
*/

// include the SPI library:
#include <SPI.h>
#include <Wire.h>

enum {
  GPB_OE = 0x04,
  GPB_ODATA,
  GPB_IDATA,

  GPE_OE = 0x10,
  GPE_ODATA,
  GPE_IDATA,
};

const byte WRITE = 0b10000000;   // SPI2GPIO write

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = 10;
const int resetPin       =  9;

/*read register*/
unsigned regRead(int address) {
  unsigned v;

  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin, LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address | 0x0);
  v = SPI.transfer(0x0);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin, HIGH);
  return v;
}

/*write register*/
unsigned regWrite(int address, int value) {
  unsigned v;
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin, LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address | WRITE);
  v = SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin, HIGH);
  return v;
}


// the setup routine runs once when you press reset:
void setup() {
  int v;

  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  // set the slaveSelectPin as an output:
  pinMode(slaveSelectPin, OUTPUT);

  // initialize SPI:
  SPI.begin();
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE3));


  // set the reset Pin as an output
  pinMode(resetPin,       OUTPUT);

  // reset FPGA logic
  digitalWrite(resetPin, LOW);
  delay(1);
  digitalWrite(resetPin, HIGH);

  /* LED1/2 as output */
  v = regRead(GPB_OE);
  regWrite(GPB_OE, v | 0xC0);
  
  Wire.begin();
}

unsigned leds = 0x80;
// the loop routine runs over and over again forever:
void loop() { 

  unsigned v;
 
  if (0 == (regRead(GPE_IDATA) & 0x10))  //Press USER1 led will reverse
  {
     
    v = regRead(GPB_ODATA); //read LED register data 
    regWrite(GPB_ODATA, (v & 0x3F) | (leds & 0xC0));  //turn the LED reverse
    leds = ~leds;       //~LED 
  }
  
   delay(250);// wait 250ms 
}
