#ifndef serial_utils_h
#define serial_utils_h

void serialPrintHex(int value, int digits) {
  for (int digit = digits; digit--> 0; ) {
    int digitValue = value >> (digit * 4);
    Serial.print(digitValue & 0xF, HEX);
  }
}

#endif
