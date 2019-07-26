/*
  05SWITCH_FPGA_BIT

  using i2c to send infomation to ESP32 to switch the fpga-bit.
  you can see information from ESP32 Serial to see what the infomation you send.
  <top-level-directory-SDcard>/overlay/ has some fpga-bit, 
  the boot.py acquiescently load spi2gpio.bit, which depend on the boaed_config.json 
  if you want to load the other one dynamically, you can Refer to this example
  and if you want to load the other one acquiescently,
  you can change the value about overlay_on_boot in boaed_config.json 
  
  The MIT License (MIT)
  Copyright (C) 2019  Seeed Technology Co.,Ltd.
*/

// include the SPI library:
#include <SPI.h>
#include <Wire.h>

enum {    
  GPZ_OE = 0x1C,
  GPZ_ODATA,
  GPZ_IDATA,
};

const byte WRITE = 0b10000000;   // SPI2GPIO write

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

  /*
   * Enable TXS0104E-1 for SPI
   * Enable TXS0104E-0 for UART & I2C
   */
  regWrite(GPZ_OE,    0xE0);
  /*
   * FPGA_AR_OE2    = High
   * FPGA_AR_OE1    = High
   * FPGA_ESP_IN12  = Low, Enable ESP32 I2C
   */
  regWrite(GPZ_ODATA, 0xC0);

  Wire.begin();
}

// the loop routine runs over and over again forever:
void loop() {
  int r;
  
  Wire.beginTransmission(0x20); // Send from machine address and start bit
  Wire.write(0x01);             // Send register address
  Wire.write(0x02);             // send data
  /* 
   * 0x02 ->spi2gpio
   * 0x01 ->hdmi_v1
   * 0x00 ->mipi_camera
   */
  Wire.write(0x5A);             // send data
  Wire.endTransmission();       // send stop bit

  Serial.println("successful");
 
  // delay in between reads for stability
  delay(1500);
}
