#ifndef HELPFUNC
#define HELPFUNC
#include <string.h>
#include <stdio.h>
#include <utility/W5100.h>

//Function Declerations
void setIP(uint8_t ip , char * valueStr);
void SWreset();
void ShowSockStatus();
void restoreDef();
void setMAC(char * valueStr);
byte charsTo1Byte(char A , char B);
void resetSysTimout();
//end of function declerations


void setMAC(char * valueStr) {
  byte tmpMAC[6];
  char *newstr;
  Serial.println(valueStr);
  newstr = strdup(valueStr);
  Serial.println(newstr);
  for (size_t i = 0 ; i < 6; i++) {
    //   Serial.print(valueStr[i * 2]);  Serial.print(valueStr[(i * 2)+1]); Serial.print(" ");

    tmpMAC[i] = charsTo1Byte(valueStr[i * 2], valueStr[(i * 2) + 1]);
    //  Serial.println(tmpMAC[i]);
    //   if (tmpMAC[i] == -1) return;
  }
  for (size_t i = 0 ; i < 6; i++) {
    //  Serial.print("tnetConf.mac[");Serial.print(i);Serial.print("]: "); Serial.println(netConf.mac[i]);
    //  Serial.print("tmpMAC[");Serial.print(i);Serial.print("]: "); Serial.println(tmpMAC[i]);
    sys.mac[i] = tmpMAC[i];
  }
}


byte charsTo1Byte(char A , char B) { // convertor for mac address 0xAB = A*16+B
  byte num;

  if (A > 64 && A < 71)num = (A - 55) * 16;
  else if ( A > 47 && A < 58) num = (A - 48) * 16;
  //  else return -1;
  if (B > 64 && B < 71)num = num - 55 + B ;
  else if ( B > 47 && B < 58) num = num - 48 + B ;
  //  else return -1;
  return num;
}

//set ip address from web server text box post
void setIP(uint8_t ip, char * valueStr)
{
  IPAddress newIP;
  uint16_t num;
  uint8_t oct;
  uint8_t c;
  oct = 0;
  num = 0;
  c = 0;

  while (valueStr[c] != '\0') {

    if (valueStr[c] != '.') {
      //  Serial.print(valueStr[c]);
      num = num * 10;
      num += (valueStr[c] - '0');
    }
    else if (oct < 4) {
      //  Serial.print("c:"); Serial.println(c);
      //   Serial.print("oct:"); Serial.println(oct);
      //   Serial.print("num:"); Serial.println(num);
      newIP[oct] = num;
      oct++;
      num = 0;
    }
    else {
      return;
    }
    c++;
  }
  if (oct < 4) {
    newIP[oct] = num;
  }

  switch (ip) {
    case IP_ADD:         {
        netConf.ip = newIP;
        break;
        Serial.println(netConf.ip);
      }
    case MASK:       {
        netConf.mask = newIP;
        break;
      }
    case GATEWAY:    {
        netConf.gateway = newIP;
        break;
      }
    case TIME_SERVER: {
        netConf.timeServer = newIP;
        break;
      }
    case DNS:        {
        netConf.dns = newIP;
        break;
      }
    default: break;
  }
  //  Serial.print("To set: "); Serial.print(newIP[0]); Serial.print(newIP[1]); Serial.print(newIP[2]); Serial.print(newIP[3]);
}

void ShowSockStatus()
{
  for (int i = 0; i < MAX_SOCK_NUM; i++) {
    Serial.print("Socket#");
    Serial.print(i);
    uint8_t s = W5100.readSnSR(i);
    Serial.print(":0x");
    Serial.print(s, 16);
    Serial.print(" in:");
    Serial.print(W5100.readSnPORT(i));
    Serial.print(" D:");
    uint8_t dip[4];
    W5100.readSnDIPR(i, dip);
    for (int j = 0; j < 4; j++) {
      Serial.print(dip[j], 10);
      if (j < 3) Serial.print(".");
    }
    Serial.print(" ex(");
    Serial.print(W5100.readSnDPORT(i));
    Serial.println(")");
  }
}

void SWreset() {
  Serial.println("SW Reset..");
  delay(2000);
  digitalWrite(SW_RST_PIN, LOW); //pin 6
}

void userReset() {
  Serial.println();
  Serial.print("User system reset....");
  digitalWrite(FACTORY_RST_LED, HIGH); //turn led on
  //load defaults settings to structure
  //  loadSys(); //system configurations must be first
  loadNet(); //network configurations
  initAnalog();
  initDigital();
  initRelays();
  //  initRules(); //must be after initAnalog(); and initDigital();
  initOneWire();
  initRules(); //must be after initAnalog(); and initDigital();

  //save stractures on eepprom
  //  netStart = EEPROM_writeAnything(0, sys);
  oneWireStart = EEPROM_writeAnything(0, netConf);
  analogStart = EEPROM_writeAnything(oneWireStart , oneWire);
  digitalStart = EEPROM_writeAnything(analogStart , analog);
  relayStart = EEPROM_writeAnything(digitalStart, digital);
  ruleStart = EEPROM_writeAnything(relayStart, relay);
  sysStart = EEPROM_writeAnything(ruleStart, rule);
  // EEPROM_writeAnything(sysStart, sys);

  //done restoreing defaults
  Serial.println("Done!");
  digitalWrite(FACTORY_RST_LED, false);   //turn led off
  SWreset();   //reset the system
}

void factoryReset() {
  Serial.print("Factory system reset..");
  digitalWrite(FACTORY_RST_LED, HIGH); //turn led on
  //load defaults settings to structure
  loadSys(); //system configurations must be first
  loadNet(); //network configurations
  initAnalog();
  initDigital();
  initRelays();
  //  initRules(); //must be after initAnalog(); and initDigital();
  initOneWire();
  initRules(); //must be after initAnalog(); and initDigital();

  //save stractures on eepprom
  // netStart = EEPROM_writeAnything(0, sys);
  oneWireStart = EEPROM_writeAnything(0, netConf);
  analogStart = EEPROM_writeAnything(oneWireStart , oneWire);
  digitalStart = EEPROM_writeAnything(analogStart , analog);
  relayStart = EEPROM_writeAnything(digitalStart, digital);
  ruleStart = EEPROM_writeAnything(relayStart, relay);
  sysStart = EEPROM_writeAnything(ruleStart, rule);
  EEPROM_writeAnything(sysStart, sys);

  //done restoreing defaults
  Serial.println("Done!");
  digitalWrite(FACTORY_RST_LED, false);   //turn led off
  SWreset();   //reset the system
}

void resetSysTimout() {
  lastTimeStamp = now();
}

void checkReset() {
  if (sys.is_After_reset) {
    sys.is_After_reset = false;
    EEPROM_writeAnything(sysStart, sys);
  }
  else {
//    Serial.println(sys.is_After_reset);
    sys.is_After_reset = true;
  //  Serial.println(sys.is_After_reset);
    EEPROM_writeAnything(sysStart, sys);
    SWreset();
  }
}

#endif
