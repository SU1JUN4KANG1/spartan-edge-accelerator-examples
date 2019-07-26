/*
  05SWITCH_FPGA_BIT

  using i2c to switch the fpga-bit.
  you can use Serial to see whether the i2c is working.
  <top-level-directory-SDcard>/overlay has some fpga-bit, 
  the boot.py acquiescently load spi2gpio.bit.
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
  GPC_OE = 0x08,
  GPC_ODATA,
  GPC_IDATA,
  #define GPC_ALT_UART_TX   0x01
  #define GPC_ALT_UART_RX   0x02
  #define GPC_ALT_UART_MASK (GPC_ALT_UART_TX | GPC_ALT_UART_RX)
  GPC_ALT,

  GPD_OE = 0x0C,
  GPD_ODATA,
  GPD_IDATA,
  
  UART_DATA = 0x18,
  #define UART_STAT_TX_BUSY  0x10
  #define UART_STAT_RX_DV    0x01
  UART_STAT,

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

  /* UART alternate pin */
  regWrite(GPC_ALT, GPC_ALT_UART_MASK);

  regWrite(GPC_OE, 0xFF);
  regWrite(GPD_OE, 0x01);

  Wire.begin();
}

/*
 * looking for i2c device
 * if success return 0
 * if fail return -1
 */
int i2c_scan(void) {
  byte error, address;
  int nDevices;
  byte dev_0x20 = 0;
  byte dev_0x6B = 0;

  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();  // receive the results

    if (error == 0) // success,the address mounted i2c device
    {
      Serial.print("0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);  // output the address by Serial
      Serial.print(" ");

      nDevices++;
      if (address == 0x20) {
        dev_0x20 = 1;
      }
      if (address == 0x6B) {
        dev_0x6B = 1;
      }
    }
    else if (error==4)  
    {
      Serial.print("Unknow error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0) {  // there are no i2c device in those address
    Serial.println("No I2C devices found\n");
    return -1;
  }
  if (dev_0x20 && dev_0x6B) { // i2c device already found
    return 0;
  }
  return -1;
}

int flag1 = 0; // to avoid circulating  
void loop() {
  int r;
  
  if(flag1 == 0)
  {
      Serial.print("I2C : ");
      r = i2c_scan(); //looking for i2c device
      if (r < 0) {
        Serial.print("FAIL ");
        Serial.println(r);
      } else {
        flag1 =1;
        Serial.println("OK");
      }

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
   }

  // delay in between reads for stability
  delay(1500);
}
