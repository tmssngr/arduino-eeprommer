#include "serial_utils.h"
#include "data_jute6k_video.h"
//#include "data_ub8830_jute6k.h"
//#include "data_ub8830_jute6k_modified_keyboard.h"

const uint8_t PIN_SER = 8;
const uint8_t PIN_OE = 9;
const uint8_t PIN_RCLK = 10;
const uint8_t PIN_SERCLK = 11;
const uint8_t PIN_D0 = 0;
const uint8_t PIN_D1 = 1;
const uint8_t PIN_D2 = 2;
const uint8_t PIN_D3 = 3;
const uint8_t PIN_D4 = 4;
const uint8_t PIN_D5 = 5;
const uint8_t PIN_D6 = 6;
const uint8_t PIN_D7 = 7;

const uint8_t DEBUG = 0;

void setup() {
  pinMode(PIN_SER, OUTPUT);
  pinMode(PIN_OE, OUTPUT);
  pinMode(PIN_RCLK, OUTPUT);
  pinMode(PIN_SERCLK, OUTPUT);
  setDataMode(INPUT);
  setAddress_WE_OE_CS(0, 1, 1, 1);

  Serial.begin(9600);
  while (!Serial) {
    // wait
  }

  Serial.println("start" );

  // uncomment the next 2 lines to write and verify the imported DATA array
  //writeData();
  //verifyData();

  printRomContent(0, 0x2000);
}

void loop() {
}

void testWriteRead() {
  for (int i = 0; i < 0x800; i++) {
    write(i, 0xff);
  }

  printRomContent(0, 0x800);
}

uint8_t readData(int address) {
  return pgm_read_byte(DATA + address);
}

void writeData() {
  Serial.println("Writing");

  for (int i = 0; i < DATA_LENGTH; i++) {
    if (i % 16 == 0) {
      if (i > 0) {
        Serial.println();
      }
      serialPrintHex(i, 4);
      Serial.print(" ");
    }

    writeAndVerify(i, readData(i));
  }
}

void verifyData() {
  Serial.println("Verifying");

  int errors = 0;
  for (int i = 0; i < DATA_LENGTH; i++) {
    if (i % 16 == 0) {
      if (i > 0) {
        Serial.println();
      }
      serialPrintHex(i, 4);
    }

    uint8_t readValue = read(i);
    uint8_t expectedValue = readData(i);
    if (readValue != expectedValue) {
      Serial.print(" expected ");
      serialPrintHex(expectedValue, 2);
      Serial.print(" at ");
      serialPrintHex(i, 4);
      Serial.print(", but read ");
      serialPrintHex(readValue, 2);
      Serial.println();
      errors++;
    }

    if (i % 16 == 15 || i + 1 == DATA_LENGTH) {
      Serial.println(" OK");
    }
  }

  if (errors == 0) {
    Serial.println("No errors :)");
  }
  else {
    Serial.print(errors);
    Serial.println(" errors :(");
  }
}

void printRomContent(int start, int end) {
  for (int i = start; i < end; i++) {
    if (i % 16 == 0) {
      if (i > 0) {
        Serial.println();
      }
      serialPrintHex(i, 4);
    }

    Serial.print(" ");

    uint8_t value = read(i);
    serialPrintHex(value, 2);
  }
  Serial.println();
}

void writeAndVerify(uint16_t address, uint8_t value) {
  if (DEBUG == 1) {
    serialPrintHex(address, 4);
    Serial.print(":=");
    serialPrintHex(value, 2);
    Serial.println(">");
  }

  uint8_t existingValue = read(address);
  if (existingValue == value) {
    Serial.print(" ");
    return;
  }

  write(address, value);
  delay(10);

  for (int i = 0; i < 10; i++) {
    uint8_t readValue = read(address);
    if (readValue == value) {
      return;
    }

    Serial.print(".");
  }

  Serial.print("FAILURE at ");
  serialPrintHex(address, 4);
  Serial.print(" - aborting");
  while (true) {
  }
}

void write(uint16_t address, uint8_t value) {
  setAddress_WE_OE_CS(address, 1, 1, 1);
  // In the XL1816 the write cycle is initiated by applying 
  // a logical 0 to both /WE adn /CE while /OE is logical 1.
  // The address inputs are latched into the device on the 
  // falling edge of /WE or /CE (whichever is last) to specify 
  // the address that is to be written.
  setAddress_WE_OE_CS(address, 0, 1, 0);
  setDataMode(OUTPUT);
  writeByte(value);
  // Data on the I/O pins is then latched into the device by
  // bringing either /WE or /CE high.
  setAddress_WE_OE_CS(address, 1, 1, 1);
  // Once the data is latched, the XL2816 will automatically 
  // erase the selected byte and write the new data in less
  // than 10ms.
  // The system is therefore freed to proceed with other operations 
  // while the XL2816 autnomously executes its internal write cycle.
  // The I/O pins will be in a high impedance state while the write
  // operation is in progress with the exception of I/O7 if a read 
  // command is asserted.
  setDataMode(INPUT);
}

void writeByte(uint8_t value) {
  digitalWrite(PIN_D0, (value & 1 << 0) != 0);
  digitalWrite(PIN_D1, (value & 1 << 1) != 0);
  digitalWrite(PIN_D2, (value & 1 << 2) != 0);
  digitalWrite(PIN_D3, (value & 1 << 3) != 0);
  digitalWrite(PIN_D4, (value & 1 << 4) != 0);
  digitalWrite(PIN_D5, (value & 1 << 5) != 0);
  digitalWrite(PIN_D6, (value & 1 << 6) != 0);
  digitalWrite(PIN_D7, (value & 1 << 7) != 0);
}

uint8_t read(uint16_t address) {
  setDataMode(INPUT);
  setAddress_WE_OE_CS(address, 1, 1, 1);
  setAddress_WE_OE_CS(address, 1, 0, 0);
  uint8_t value = readByte();
  setAddress_WE_OE_CS(address, 1, 1, 1);
  return value;
}

uint8_t readByte() {
  return digitalRead(PIN_D0) << 0
       | digitalRead(PIN_D1) << 1
       | digitalRead(PIN_D2) << 2
       | digitalRead(PIN_D3) << 3
       | digitalRead(PIN_D4) << 4
       | digitalRead(PIN_D5) << 5
       | digitalRead(PIN_D6) << 6
       | digitalRead(PIN_D7) << 7;
}

void setDataMode(uint8_t mode) {
  pinMode(PIN_D0, mode);
  pinMode(PIN_D1, mode);
  pinMode(PIN_D2, mode);
  pinMode(PIN_D3, mode);
  pinMode(PIN_D4, mode);
  pinMode(PIN_D5, mode);
  pinMode(PIN_D6, mode);
  pinMode(PIN_D7, mode);
}

void setAddress_WE_OE_CS(uint16_t address, uint8_t we, uint8_t oe, uint8_t cs) {
  uint16_t value = address & 0x1FFF;
  if (we != 0) {
    value |= 0x2000;
  }
  if (oe != 0) {
    value |= 0x4000;
  }
  if (cs != 0) {
    value |= 0x8000;
  }
  shift16(value);
}

void shift16(uint16_t value) {
  if (DEBUG == 1) {
    Serial.print("value = ");
    Serial.println(value, BIN);
  }

  // the serial registers are connected in the wrong order
  // so we need to switch here the bytes

  digitalWrite(PIN_OE, 0);
  digitalWrite(PIN_RCLK, 0);
  digitalWrite(PIN_SERCLK, 0);

  shift8(value);
  shift8(value >> 8);

  digitalWrite(PIN_RCLK, 1);
  delayMicroseconds(1);
  digitalWrite(PIN_RCLK, 0);
  delayMicroseconds(1);
  digitalWrite(PIN_OE, 0);
  delayMicroseconds(1);
}

void shift8(uint16_t value) {
  for (int i = 0; i < 8; i++) {
    const uint8_t bit = (value & 0x80) != 0 ? 1 : 0;

    if (DEBUG == 1) {
      Serial.print(i);
      Serial.print("=");
      Serial.println(bit);
    }

    digitalWrite(PIN_SER, bit);
    delayMicroseconds(1);
    digitalWrite(PIN_SERCLK, 1);
    delayMicroseconds(1);
    digitalWrite(PIN_SERCLK, 0);
    delayMicroseconds(1);

    value <<= 1;
  }
}
