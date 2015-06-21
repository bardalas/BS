//#include <dht.h>
#include <Base64.h>
#include <SD.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Time.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <WebServer.h>
#include <OneWire.h>
#include <TimerOne.h>
#include <EEPROM.h>
#include <String.h>

//============================ Function Declerations ======================

time_t syncTime(); //get NTP time and store to system time
void initSNMP();   //initelize SNMP service

//TODO: add here all functions from all of the following includes

//=========================================================================


#include <SerialSNMP.h>
#include "pStrings.h" //stores strings into memory
#include "EEPROMAnything.h"
#include "Timer.h"
#include "System.h"
#include "analog.h"
//#include "humid.h"
#include "digital.h"
#include "relay.h"
#include "oneWireSensor.h"
#include "rules.h"
#include "helpFunc.h"
#include "logger.h"
#include "NTP.h"
#include "webPages.h"
#include "SNMP.h"
#include "modBus.h"


void setup() {
  Serial.begin(9600);


  lastVisit = millis();
  loggerVisit = millis();

  initPort(); //configure the I/O pins

  if (digitalRead(FACTORY_RST_PIN) == LOW)  factoryReset(); //Factory System Reset - reset all parameters
  
  if (digitalRead(USER_RST_PIN) == LOW)     userReset();    //User System Reset - reset all parameters excluding sys[]

  loadEEPROM(); //load values from eeprom to ram memory
  
  checkReset(); //make one more SW-reset after booting the system - work around for w5100 tri-state problem
  
  initTimer(&LOG_1M_TMR, 60); // 1 minute timer for log process
  initTimer(&EE_SAVE_TMR, 5); // 5 seconds eeprom delay time
  resetLogTicks();
  initLogSensor();
  initSD(); //must be after loadSys();
  initEthernet(); //Start ethernet connection
  initSysTime(); //Set the system time - must be after initEthernet();
  initWebServer(); //server // client
  initSNMP(); //udp
  // initModBus(); //server // client
  oneWireRead = 0;
  resetSysTimout(); //reset the SNMP watchdog timer
}



// ==========================================================================================================
//                                                 Main Loop
// ==========================================================================================================
void loop() {

  updateDigital();

  updateAnalog();

  oneWireProccess();

  applyRules(); //check all rules one by one and set the relays accordingly

  //1 minute timer - Mark the sensors that needs to be save. Each sensor might have a different time interval
  if (updateTimer(&LOG_1M_TMR)) checkLogTicks();

  //SD Card saving process for sensors marked by the **checkLogTicks();** function
  logProccess();

  //eeprom write every 5 seconds only if requsted
  if (updateTimer(&EE_SAVE_TMR)) eeSaveHandler(); //can we drop this ?

  //check if syncInterval is due and sync the clock
  now();

  //process WEB incoming connections
  webserver.processConnection();

  //process SNMP incomming packets
  listenSNMP();

  //process MODBUS incomming connections
  //  listenModBus();

  //system heartbeat led
  blinkLed();

  //system network keep alive
  //Sotware Watchdog - SNMP request will reset the watchdog
  if (now() - lastTimeStamp > sys.srto)  SWreset();
}
// ==========================================================================================================
//                                                    End main loop
// ==========================================================================================================




// ========================================================
//                 I/O Pins port configuration
// =========================================================
void initPort() {
  //System Pins
  pinMode(SW_RST_PIN, OUTPUT); //resetPin needs to connect to RESET pin of arduino's using a resistor inline
  digitalWrite(SW_RST_PIN, HIGH); // Set the pin to high so we don't reset us on system startup
  pinMode(ETH_CS, OUTPUT);
  pinMode(FACTORY_RST_PIN, INPUT_PULLUP);
  pinMode(USER_RST_PIN, INPUT_PULLUP);
  pinMode(FACTORY_RST_LED, OUTPUT); //led indicator
  pinMode(HEART_BEAT, OUTPUT); //

  //Sensors Pins
  //Analog Pins 54-62 = A8-A0
  for (int i = 0; i < MAX_ANALOG; i++)     pinMode(ANALOG_START_PIN + i, INPUT_PULLUP);

  //Digital Pins 22-30
  for (int i = 0; i < MAX_DIGITAL; i++)    {
    pinMode(DIGITAL_START_PIN + i, INPUT_PULLUP);  //Digital Pins 22-30
    last_state[i] = LOW;
  }

  //Relay Pins 40-48
  for (int i = 0; i < MAX_RELAY; i++) {
    pinMode(RELAY_START_PIN + i, OUTPUT);
    setRelayToNormaly(i); //Realy pins can be configured by user to normaly High or Normaly Closed
  }
}


// ====================================================================
//                Set system Time and sync Provider
// ====================================================================
void initSysTime() {
  // setTime(sys.defaultTime); //set default time to 12:00
  setTime(00, 00, 00, 1, 1, 2015); //set default time to 01/01/2015 00:00:00 //12:00 AM
  setSyncProvider(syncTime);
  setSyncInterval(netConf.NTPSyncInterval * 60); //sync interval in seconds
}


// ====================================================================
//                  System Time Sync Handler
// ====================================================================
time_t syncTime() { //Try getting the current time from NTP server. If no NTP respond set default time and date

  uint32_t beginWait = millis();

  //Try getting NTP time from Server
  time_t time = getNtpTime();
  Serial.print("NTP Sync Interval: "); Serial.println(netConf.NTPSyncInterval);
  if (time != -1 )

    //NTP Success - return NTP time
    return time;

  //NO respons from NTP return the system time adding the time spent on trying to sync
  else return getSysTime() + ((millis() - beginWait) / 1000);
}



// ====================================================================
//                    Initelize Ethernet connection
// ====================================================================
void initEthernet() {

  boolean succsess = false;
  int dhcp_count = 0;
  if (netConf.dhcp) {
    Serial.print("Device MAC: ");
    Serial.print(sys.mac[0], 16); Serial.print(sys.mac[1], 16); Serial.print(sys.mac[2], 16); Serial.print(sys.mac[3], 16); Serial.print(sys.mac[4], 16); Serial.println(sys.mac[5], 16);
    Serial.println("Starting DHCP");
    while (!Ethernet.begin(sys.mac)) {
      if (dhcp_count < 5 ) {
        Serial.println("Retrying DHCP");
        dhcp_count++;
      }
      else SWreset();
    }

    netConf.ip = Ethernet.localIP();
    netConf.dns = Ethernet.dnsServerIP();
    netConf.gateway = Ethernet.gatewayIP();
    netConf.mask = Ethernet.subnetMask();
    //   Ethernet.begin(sys.mac); // <=== TODO: What is this for ????
    eeSave[NET];
    succsess = true;
  }

  else { //starting Static IP
    Serial.print("Starting Static IP: ");
    Ethernet.begin(sys.mac, netConf.ip , netConf.dns, netConf.gateway, netConf.mask);
    netConf.dhcp = 0; //To indicate in web that we are working on static
  }
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());

  Serial.print("DNS: ");
  Serial.println(Ethernet.dnsServerIP());
  //  Serial.print(netConf.dns[0]); Serial.print("."); Serial.print(netConf.dns[1]); Serial.print("."); Serial.print(netConf.dns[2]);Serial.print("."); Serial.println(netConf.dns[3]);

  Serial.print("GATEWAY: ");
  Serial.println(Ethernet.gatewayIP());
  //  Serial.print(netConf.gateway[0]); Serial.print("."); Serial.print(netConf.gateway[1]); Serial.print("."); Serial.print(netConf.gateway[2]); Serial.print("."); Serial.println(netConf.gateway[3]);

  Serial.print("MASK: ");
  Serial.println(Ethernet.subnetMask());
  //  Serial.print(netConf.mask[0]); Serial.print("."); Serial.print(netConf.mask[1]); Serial.print("."); Serial.print(netConf.mask[2]); Serial.print("."); Serial.println(netConf.mask[3]);

  Serial.print("Web Port: ");
  Serial.println(netConf.webPort);

  Serial.print("SNMP Port: ");
  Serial.println(netConf.snmpPort);
}


// fast sampling ports configuration
void fastSampeling(boolean) {
  //  Serial.println(F("fastSampling ON");
  Timer1.initialize(20000); // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-and-off, per second)
  Timer1.attachInterrupt( updateDigital ); // attach the service routine her
}



// =================================================================
//                System HeartBeat Led Blink
// =================================================================
void blinkLed() {
  if (millis() - lastVisit > 500) {
    digitalWrite(HEART_BEAT, !digitalRead(HEART_BEAT)); //toggle
    lastVisit = millis();
  }
}


// =================================================================
//                SD Card Log Proccess
// =================================================================
void logProccess() {
  //Checks for sensor log interval ticks, Save to SD card and reset the ticks
  //One save each main loop

  if (logger.toSave == true && (millis() - loggerVisit > 1000)) { //1 seconds between saving to SD and only if requiered
    loggerVisit = millis(); //update the time tarcker
    if (analogSaveIndex < sys.maxAlowedAnalog) {
      if (logger.analog[analogSaveIndex] == true) {
        logSensor(ANALOG, analogSaveIndex); //make the saving
        logger.analog[analogSaveIndex] = false;
      }
      analogSaveIndex++;
      return;
    }
    else analogSaveIndex = 0;

    loggerVisit = millis(); //update the time tarcker

#ifdef DHT
    if (humidSaveIndex < sys.maxAlowedHumid) {
      if (logger.humid[humidSaveIndex] == true) {
        logger.humid[humidSaveIndex] = false;
      }
      humidSaveIndex++;
      return;
    }
    else humidSaveIndex = 0;
#endif

    if (oneWireSaveIndex < sys.maxAlowedOneWire) {
      if (logger.oneWire[oneWireSaveIndex] == true) {
        logSensor(ONEWIRE, oneWireSaveIndex); //make the save
        logger.analog[oneWireSaveIndex] = false;
      }
      oneWireSaveIndex++;
      return;
    }
    else oneWireSaveIndex = 0;

    if (digitalSaveIndex < sys.maxAlowedDigital) {
      if (logger.digital[digitalSaveIndex] == true) {
        logSensor(DIGITAL, digitalSaveIndex); //make the save
        logger.digital[digitalSaveIndex] = false;
      }
      digitalSaveIndex++;
    }
    else digitalSaveIndex = 0;
  }

  if (digitalSaveIndex == 0 && oneWireSaveIndex == 0 && analogSaveIndex == 0) logger.toSave = false;
}

// ====================================================================
//                    Load EEPROM to RAM
// ====================================================================
void loadEEPROM(void) {

  // netStart = EEPROM_readAnything(0, sys);
  oneWireStart = EEPROM_readAnything(0 , netConf);
  analogStart  = EEPROM_readAnything(oneWireStart , oneWire);
  digitalStart = EEPROM_readAnything(analogStart , analog);
  relayStart   = EEPROM_readAnything(digitalStart , digital);
  ruleStart    = EEPROM_readAnything(relayStart , relay);
  sysStart     = EEPROM_readAnything(ruleStart, rule);
                 EEPROM_readAnything(sysStart, sys);
  for (uint8_t i = 0; i < 8; i++)
    eeSave[i] = 0;
}

// ====================================================================
//                    Save RAM to EEPROM
// ====================================================================
void eeSaveHandler() {

  if (eeSave[NET])         EEPROM_writeAnything(0, netConf);
  if (eeSave[ONEWIRE])     EEPROM_writeAnything(oneWireStart , oneWire);
  if (eeSave[ANALOG])      EEPROM_writeAnything(analogStart , analog);
  if (eeSave[DIGITAL])     EEPROM_writeAnything(digitalStart, digital);
  if (eeSave[RELAY])       EEPROM_writeAnything(relayStart, relay);
  if (eeSave[RULES])       EEPROM_writeAnything(ruleStart, rule);
  if (eeSave[SYS])         EEPROM_writeAnything(sysStart, sys);

  for (uint8_t i = 0; i < 8; i++)
    eeSave[i] = false;
}
