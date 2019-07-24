/*
  RGBLEDsetting

  set the RGBLED's R,G,B value,which will change the led color
  
  The MIT License (MIT)
  Copyright (C) 2019  Seeed Technology Co.,Ltd.
*/

// include the SPI library:
#include <SPI.h>
#include <Wire.h>

enum {
  SK6805_CTRL = 0x14,
  SK6805_DATA,

  WRITE_ADDR = 0b10000000,
};

// SPI2GPIO write
const byte WRITE = WRITE_ADDR;   

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = 10;
const int resetPin       =  9;

/* read register */
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

/* write register */
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
  
  Wire.begin();
}

// the loop routine runs over and over again forever:
void loop() {
  static unsigned long color = 0xFFUL;
  
  /* set RGB1 value B->R->G */
  regWrite(SK6805_CTRL, 0x0);
  regWrite(SK6805_DATA, (color >>  3) & 0x1F);// blue
  regWrite(SK6805_CTRL, 0x1);
  regWrite(SK6805_DATA, (color >> 11) & 0x1F);// red
  regWrite(SK6805_CTRL, 0x2);
  regWrite(SK6805_DATA, (color >> 19) & 0x1F);// green

  /* set RGB2 value R->G->B */
  regWrite(SK6805_CTRL, 0x3);
  regWrite(SK6805_DATA, (color >> 19) & 0x1F);// blue
  regWrite(SK6805_CTRL, 0x4);
  regWrite(SK6805_DATA, (color >>  3) & 0x1F);// red
  regWrite(SK6805_CTRL, 0x5);
  regWrite(SK6805_DATA, (color >> 11) & 0x1F);// green

  /* set color value */
  switch (color) {
  case 0x0000FFUL: color = 0x00FF00UL; break;
  case 0x00FF00UL: color = 0xFF0000UL; break;
  case 0xFF0000UL: color = 0x0000FFUL; break;
  default:
    color = 0xFFUL; break;
  }
  
  // delay in between reads for stability
  delay(1500);    
}
