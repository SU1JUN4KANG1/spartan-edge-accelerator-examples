/*
  DAC-AND-ADC testing
 
  read ADC information from FPGA and output by Serial
  write DAC value to FPGA

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
  
  DAC_DATA0 = 0x16,
  DAC_DATA1,

  ADC_DATA = 0x1F,

  WRITE_ADDR = 0b10000000,
};

const byte WRITE = WRITE_ADDR;   // SPI2GPIO write

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

  /* Enable ADC1173, set /OE to LOW */
  regWrite(GPE_OE, 0x80);
  v = regRead(GPE_IDATA);
  v &= ~0x80;
  regWrite(GPE_ODATA, v);

  Wire.begin();
}

/*read ADC_data and return Voltage*/
unsigned long /*Voltage(ms)*/readADC_data(void){
  int adc_data;
  int Voltage;
  
  //read ADC value
  adc_data = regRead(ADC_DATA); 

  //ADC_data Transform to  Voltage(ms)
  /*
   * if yu want to know detail,
   * you can come [http://www.ti.com/product/DAC7311]
   */
  Voltage =(unsigned long)adc_data * 3300 / 256; 

  return Voltage;
}

/*write Voltage(mv) to DAC*/
void writeDAC_data(unsigned long Volt_val/*Voltage(ms)*/){

  /*
   * if yu want to know detail,
   * you can come [http://www.ti.com/product/DAC7311]
   */
  // DATA1 first
  regWrite(DAC_DATA1, (Volt_val >> 2) & 0x3F);
  // DATA0 last
  regWrite(DAC_DATA0, (Volt_val << 6) & 0xC0);
}

// the loop routine runs over and over again forever:
void loop() {
  unsigned long Volt_val;

  //read ADC value
  Volt_val =readADC_data();

   /*output Voltage(ms) by Serial*/
  Serial.print("ADC : ");
  Serial.print(Volt_val);
  Serial.print(" mV ");

  //output DAC val
  // DAC-OUT = ADC-IN ,input DAC value ,which is what you read before 
  writeDAC_data(Volt_val);

  Serial.println(); //Change other line
  delay(1500);      // delay in between reads for stability
}
