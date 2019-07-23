

// include the SPI library:
#include <SPI.h>
#include <Wire.h>

enum {
  GPA_OE = 0x00,
  GPA_ODATA,
  GPA_IDATA,

  GPB_OE = 0x04,
  GPB_ODATA,
  GPB_IDATA,

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

  GPE_OE = 0x10,
  GPE_ODATA,
  GPE_IDATA,

  SK6805_CTRL = 0x14,
  SK6805_DATA,

  DAC_DATA0 = 0x16,
  DAC_DATA1,

  UART_DATA = 0x18,
  #define UART_STAT_TX_BUSY  0x10
  #define UART_STAT_RX_DV    0x01
  UART_STAT,

  GPZ_OE = 0x1C,
  GPZ_ODATA,
  GPZ_IDATA,

  ADC_DATA = 0x1F,
};

const byte WRITE = 0b10000000;   // SPI2GPIO write

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

void setup() {
  int v;

  #if 1
  Serial.begin(115200);
  #else
  pinMode(0, INPUT);
  pinMode(1, INPUT);
  #endif

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

int uart_chk(void) {
  unsigned r, v, stat;
  int i;

  // eliminate the buffer effect.
  Serial.flush();

  // FPGA UART RX
  for (r = 0x30; r < 0x3A; r++) {
    v = regRead(UART_STAT);
    if (v & UART_STAT_RX_DV)
      v = regRead(UART_DATA);

    stat = regRead(UART_STAT);

    Serial.print((const char)r);
    delay(1);

    for (i = 10000; i >= 0; i--) {
      if (regRead(UART_STAT) & UART_STAT_RX_DV) {
        break;
      }
    }
    if (i < 0) {
      return -1;
    }

    v = regRead(UART_DATA);
    if (r != v) {
      Serial.print("UART RX=");
      Serial.print(v, HEX);
      Serial.print(" AR_TX=");
      Serial.println(r, HEX);
      return -2;
    }
  }

  #if 0
  Serial.print(" UART_STAT=0x");
  if (stat < 16)
    Serial.print("0");
  Serial.print(stat, HEX);

  Serial.println();
  #endif


  // FPGA UART TX
  for (r = 0x40; r < 0x4A; r++) {
    regWrite(UART_DATA, r);

    // wait tx free
    for (i = 10000; i >= 0; i--) {
      if ((regRead(UART_STAT) & UART_STAT_TX_BUSY) == 0) {
        break;
      }
    }
    if (i < 0) {
      return -3;
    }

    v = Serial.read();
    if (r != v) {
      Serial.print("UART TX=");
      Serial.print(r, HEX);
      Serial.print(" AR_RX=");
      Serial.println(v, HEX);
      return -4;
    }
  }

  return 0;
}

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
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
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
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
    return -1;
  }
  if (dev_0x20 && dev_0x6B) {
    return 0;
  }
  return -1;
}

void loop() {
  int r;
  
  Serial.print("UART: ");
  r = uart_chk();
  if (r < 0) {
    Serial.print(" FAIL ");
    Serial.println(r);
  } else {
    Serial.println(" OK");
  }

  Serial.print("I2C : ");
  r = i2c_scan();
  if (r < 0) {
    Serial.print("FAIL ");
    Serial.println(r);
  } else {
    Serial.println("OK");
  }

  Serial.println();
  delay(1500);
}
