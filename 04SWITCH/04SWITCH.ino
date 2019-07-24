/*
  switch testing

  check the switch and ouput infomation by Serial
  
  The MIT License (MIT)
  Copyright (C) 2019  Seeed Technology Co.,Ltd.
*/

// include the SPI library:
#include <SPI.h>
#include <Wire.h>

enum {
  GPE_OE = 0x10,
  GPE_ODATA,
  GPE_IDATA,

  WRITE_ADDR =0b10000000,

  is_key1 = 0x01, 
  is_key2 = 0x02,
  is_key3 = 0x04,
  is_key4 = 0x08,
};

const byte WRITE = WRITE_ADDR;   // SPI2GPIO write

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = 10;
const int resetPin       =  9;

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

/*check the switch*/
int switch_chk(void) {
  unsigned v;

  //read four switchs data
  v = regRead(GPE_IDATA);
  if ((v & 0x7F) != 0x70) //if read error ,retun -1
    return -1;

  Serial.print("Switch on K1 ");
  for (;;)  if (0 != (regRead(GPE_IDATA) & is_key1)) break;//loop forever untill key1 Switched
  Serial.println("OK");

  Serial.print("Switch on K2 ");
  for (;;)  if (0 != (regRead(GPE_IDATA) & is_key2)) break;//loop forever untill key2 Switched
  Serial.println("OK");

  Serial.print("Switch on K3 ");
  for (;;)  if (0 != (regRead(GPE_IDATA) & is_key3)) break;//loop forever untill key3 Switched
  Serial.println("OK");

  Serial.print("Switch on K4 ");
  for (;;)  if (0 != (regRead(GPE_IDATA) & is_key4)) break;//loop forever untill key4 Switched
  Serial.println("OK");

  return 0;
}

static int switch_checked = 0;  //avoiding checking looping
// the loop routine runs over and over again forever:
void loop() {
  int r;

  /*switch checking and output info by Serial*/
  if (switch_checked == 0) {
    Serial.print("Switch : ");
    Serial.println();
    r = switch_chk();   //checking
    if (r < 0) {
      Serial.print("FAIL ");
      Serial.println(r);
    } else {
      Serial.println("OK");
    }
    switch_checked = 1; //stop checking
  }

  Serial.println(); //change a line
  delay(1500);      // delay 1.5 s
}
