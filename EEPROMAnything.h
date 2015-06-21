#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
//  Serial.print("Start of address: "); Serial.println(ee);
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++) {
//    cli(); //disable interupts
    EEPROM.write(ee++, *p++);
//    sei(); //enable interupts
  } //EEPROM.update
// Serial.print("End of address: "); Serial.println(ee);
  return ee;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
  //     noInterrupts();
  //Serial.print("start of read addres:");
  //	Serial.println(ee);
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  //	Serial.print("end of read addres:");
  //	Serial.println(ee);
  // interrupts();
  return ee;
}

