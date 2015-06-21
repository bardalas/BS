#ifndef ONEWIRESENSOR
#define ONEWIRESENSOR
//  oneWire configuration and data
#define MAX_ONE_WIRE_DESCRIPTION 10 //16+1 for '\0'
#define DS_PIN 12
#define OW_ERR_TIME_OUT_MS   1800000 //30minutes

uint32_t timer=0;

struct OneWireConf {
  // uint8_t dummyS[4]; // empty byte for keepeing space between SRAM memory locations
  uint8_t Address[MAX_ONE_WIRE][8]; //address of each oneWire
  uint8_t Type[MAX_ONE_WIRE];
  uint8_t logInterval[MAX_ONE_WIRE];
  uint8_t logMinuteTick[MAX_ONE_WIRE];
  float ValueTemp[MAX_ONE_WIRE];
  float Offset[MAX_ONE_WIRE];
  float Hister[MAX_ONE_WIRE];
  boolean isFree[MAX_ONE_WIRE];
  char Description[MAX_ONE_WIRE][MAX_ONE_WIRE_DESCRIPTION];
  // uint8_t dummyE[4]; // empty byte for keepeing space between memory locations
}
oneWire;

boolean isEquale(uint8_t  Add1[], uint8_t Add2[]);
void scanOneWire();
void updateOneWire(uint8_t i);
void updateOneWireAll();

OneWire  ds(DS_PIN);

uint8_t dataOW[12];
//uint8_t poolAdd = 0;//amount.oneWire;
//boolean finished = true;
uint8_t present = 0;
//float celsius, fahrenheit;
uint8_t oneWireRead;


void initOneWire() {
  //init struct
  for (uint8_t i = 0; i < MAX_ONE_WIRE; i++) {
    for (uint8_t j = 0; j < 8 ; j++)
      oneWire.Address[i][j] = 0; //address of each oneWire
    //   oneWire.DataTemp[i] = 0;
    oneWire.ValueTemp[i] = 0;
    oneWire.Offset[i] = 0;
    oneWire.Hister[i] = 0;
    oneWire.isFree[i] = true;
    oneWire.Type[i] = 0;
    sprintf(oneWire.Description[i], "OneWire %d", i );
    oneWire.logInterval[i] = 0;
    oneWire.logMinuteTick[i] = 0;
  } // done init struct
}

void scanOneWire() {
  ds.reset();
  uint8_t addr[8];
  while (ds.search(addr)) {
    //    Serial.print("Found address: ROM =");
    /*   for (uint8_t i = 0; i < 8; i++) {
         Serial.print(addr[i], HEX);
         Serial.write(':');
       }
       //    Serial.println();
    */
    if (OneWire::crc8(addr, 7) != addr[7]) {
      //         Serial.println("CRC is not valid!");
    }

    //check if we already saved this device
    boolean known = false;
    uint8_t i = 0;
    while (i < sys.maxAlowedOneWire) { //check the array if we already saved this device somewhere
      if (isEquale(oneWire.Address[i], addr)) { //compare HEX address - return true if equale
        known = true;
        break; //i holds the index of the found address
      }
      i++;
    }
    if (!known) {//new address - save in memory
      i = 0;
      while (i < sys.maxAlowedOneWire) { //seek for empty spot
        if (oneWire.isFree[i]) { // slot i is isFree - we can save a new device here
          for (uint8_t j = 0; j < 8; j++)
            oneWire.Address[i][j] = addr[j]; //save address uint8_ts
          oneWire.isFree[i] = false; //mark this slot is taken
          // the first ROM uint8_t indicates which chip
          switch (addr[0]) {
            case 0x10:
              oneWire.Type[i] = 1;
              break;//    Chip = DS18S20  or old DS1820
            case 0x28:
              oneWire.Type[i] = 2;
              break;//    Chip = DS18B20
            case 0x12:
              oneWire.Type[i] = 3;
              break;//    Chip = DS2406(2407)
            case 0x29:
              oneWire.Type[i] = 4;
              break;//    Chip = DS2408
            case 0x1D:
              oneWire.Type[i] = 5;
              break;//    Chip = DS2423
            case 0x26:
              oneWire.Type[i] = 6;
              break;//    Chip = DS2438
            case 0x20:
              oneWire.Type[i] = 7;
              break;//    Chip = DS2450
            default:
              oneWire.Type[i] = 0;
              break;//    Unknown device
          }
          break; //saved a new device, stop looking for isFree slot
        }
        i++;
      } //the slot is not isFree - seek for next isFree slot to save addresss
    }
    else
      // Serial.println("This device is already on memory - not saved")
      ;
  } //search for next address

  // Serial.println("No more addreses");
  ds.reset_search();
  //  EEPROM_writeAnything(oneWireStart + MEM_OFFSET, oneWire);
  return;
}

void updateOneWireAll() {
  for (uint8_t i = 0; i < sys.maxAlowedOneWire; i++) {
    if (!oneWire.isFree[i]) {//ignor Free slots
      updateOneWire(i);
    }
  }
}


void oneWireProccess() {
  if (oneWireRead < sys.maxAlowedOneWire) {
    //in the range of allowed sensors
    if (!oneWire.isFree[oneWireRead]) { //is there anything to read ?
      updateOneWire(oneWireRead);
    }
    //progress the index
    oneWireRead++;
  }
  else  {
    //we reached the end of allowed sensors
    //reset the index
    oneWireRead = 0;
  }
}

void updateOneWire(uint8_t i) {
  ds.select(oneWire.Address[i]); //~5.2ms
  ds.write(0x44, 0); //tell the sensor to start temp convertions ~0.6ms

  //  delay(10);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset(); //~1ms

  ds.select(oneWire.Address[i]); //5.2ms
  ds.write(0xBE);  //0.6ms       // Read Scratchpad

  //12.6 until here, no number depended

  //  digitalWrite(19, HIGH);
  boolean valid = false;
  for ( uint8_t j = 0; j < 9 ; j++) {           // we need 9 uint8_ts
    dataOW[j] = ds.read(); //0.5mS
    if (dataOW[j] != 0) valid = true;
  }

  uint8_t cfg ;
  int16_t raw = (dataOW[1] << 8) | dataOW[0];

  switch (oneWire.Type[i]) { //0.02ms 20uSec
    case 1:
      raw = raw << 3;
      if (dataOW[7] == 0x10)raw = (raw & 0xFFF0) + 12 - dataOW[6];
      //    oneWire.DataTemp[i] = raw / 16.0;
      oneWire.ValueTemp[i] = (raw / 16.0) + oneWire.Offset[i];
      break;//    Chip = DS18S20  or old DS1820
    case 2:
      cfg = (dataOW[4] & 0x60);
      if (cfg == 0x00) raw = raw & ~7;
      else if (cfg == 0x20) raw = raw & ~3;
      else if (cfg == 0x40) raw = raw & ~1;
      //  oneWire.DataTemp[i] = raw / 16.0;
      oneWire.ValueTemp[i] = (raw / 16.0) + oneWire.Offset[i];
      break;//    Chip = DS18B20
    case 6:
      break;//    Chip = DS2438V
    default :
      oneWire.ValueTemp[i] = -1;
      break;//    Unknown device
  }
  //  oneWire.ValueTemp[i] = oneWire.DataTemp[i] + oneWire.Offset[i];

  ds.reset(); //1ms

  if ( (oneWire.ValueTemp[i] == -0.5) || (oneWire.ValueTemp[i] == 85.0) ) {
    if (timer == 0) {
      timer = millis(); // start the timer
    }
    else if ( (millis() - timer) > OW_ERR_TIME_OUT_MS ) {
      //timer is due - time to take action !!
      oneWire.ValueTemp[i] = (float)9999.0;
    }
    else {
      //timer found but it's not time yet so just progress the timer
      //nothing to progress... now() is progressing by library
    }
  }
  else {
    //no problem found - the value is valid - reset the timer
    timer=0;
  }

}


boolean isEquale(uint8_t  Add1[], uint8_t Add2[]) {
  for (uint8_t i = 0; i < 8; i++) {
    if (Add1[i] != Add2[i])
      return false;
  }
  return true;
}

void oneWireRelease(uint8_t i) {
  for (uint8_t j = 0; j < 8 ; j++)
    oneWire.Address[i][j] = 0; //address of oneWire
  //  oneWire.DataTemp[i] = 0;
  oneWire.ValueTemp[i] = 0;
  oneWire.Offset[i] = 1;
  oneWire.Hister[i] = 1;
  oneWire.isFree[i] = true;
  oneWire.Type[i] = 0;
  sprintf(oneWire.Description[i], "OneWire %d", i );
  oneWire.logInterval[i] = 0;
  oneWire.logMinuteTick[i] = 0;
}
#endif
