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

void pr_hex0(unsigned v) {
  if (v < 4096)
    Serial.print("0");
  if (v < 256)
    Serial.print("0");
  if (v < 16)
    Serial.print("0");
  Serial.print(v, HEX);
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

  /* Prevent initial error, if reset pin not used */
  // regRead(0x0);
  
  /*
   * SK6805 RGB1
   * B R G = 0x123456
   */
  regWrite(SK6805_CTRL, 0);
  regWrite(SK6805_DATA, 0x12);
  regWrite(SK6805_CTRL, 1);
  regWrite(SK6805_DATA, 0x34);
  regWrite(SK6805_CTRL, 2);
  regWrite(SK6805_DATA, 0x56);

  #if 1
  for (int i = 0; i < 3; i++) {
    regWrite(SK6805_CTRL, i);
    v = regRead(SK6805_DATA);
    Serial.print("COLOR[");
    Serial.print(i);
    Serial.print("] = 0x");
    pr_hex0(v);Serial.println();
  }
  #endif
  
  Wire.begin();
}

int sk6805_blink(void) {
  static unsigned long color = 0xFFUL;

  regWrite(SK6805_CTRL, 0x0);
  regWrite(SK6805_DATA, (color >>  3) & 0x1F);
  regWrite(SK6805_CTRL, 0x1);
  regWrite(SK6805_DATA, (color >> 11) & 0x1F);
  regWrite(SK6805_CTRL, 0x2);
  regWrite(SK6805_DATA, (color >> 19) & 0x1F);

  regWrite(SK6805_CTRL, 0x3);
  regWrite(SK6805_DATA, (color >> 19) & 0x1F);
  regWrite(SK6805_CTRL, 0x4);
  regWrite(SK6805_DATA, (color >>  3) & 0x1F);
  regWrite(SK6805_CTRL, 0x5);
  regWrite(SK6805_DATA, (color >> 11) & 0x1F);

  switch (color) {
  case 0x0000FFUL: color = 0x00FF00UL; break;
  case 0x00FF00UL: color = 0xFF0000UL; break;
  case 0xFF0000UL: color = 0x0000FFUL; break;
  default:
    color = 0xFFUL; break;
  }
  return 0;
}


void loop() {
  
  sk6805_blink();
  
  Serial.println();
  delay(1500);
}
