#ifndef SYSTEM
#define SYSTEM

#define MAX_ONE_WIRE     9
#define MAX_ANALOG       9
#define MAX_DIGITAL      9
#define MAX_RELAY        9
#define MAX_HUMID        9
#define MAX_RULES        10
#define NA               127


#define SW_RST_PIN       9   //resetPin needs to connect to RESET pin of arduino's using a resistor inline
#define FACTORY_RST_PIN  A9
#define USER_RST_PIN     11
#define FACTORY_RST_LED  18  //Output pin for indicating factory reset routine
#define HEART_BEAT       17  //as it's name, set high or low in the code for debugging
#define ETH_CS           10  //Ethernet chip select pin, need to pull high when using SD card
#define SD_CS            4   //SD card chip select pin, need to pull high when using Ethernet

#define IP_ADD           1
#define MASK             2
#define GATEWAY          3
#define TIME_SERVER      4
#define DNS              5


#define MAX_HOST_DESCRIPTION 16
#define MAX_COMMUNITY_DESCRIPTION 16

//user name and password both admin and user
//represent in base 64
#define MAX_64     22 //need to verify length, we allow max 8 chars for username and 8 for password


uint32_t lastTimeStamp = 0;

//EEPROM settings
int netStart = 0;
int sysStart = 0;
int oneWireStart = 0; //
int analogStart = 0;
int humidStart = 0;
int digitalStart = 0;
int relayStart = 0;
int ruleStart = 0;
boolean eeSave[7];
boolean last_state[MAX_DIGITAL];
boolean firstTime = true;

WebServer webserver = NULL;
EthernetServer modBus_Server(502);
EthernetClient modBus_Client;
EthernetUDP NTP_UDP;
EthernetUDP SNMP_UDP;

Timer LOG_TMR;
Timer LOG_1M_TMR;
Timer EE_SAVE_TMR;
Timer OW_ERR_TMR;
uint32_t lastVisit;
uint32_t loggerVisit;
uint8_t analogSaveIndex;
uint8_t digitalSaveIndex;
uint8_t oneWireSaveIndex;

struct systemConfiguration {
  // uint8_t maxRules; // 1 byte 1-256
  uint8_t mac[6]; //6 bytes
  uint8_t maxAlowedAnalog; // 1 byte
  uint8_t maxAlowedDigital; // 1 byte
  uint8_t maxAlowedOneWire; // 1 byte
  uint8_t maxAlowedRelay; // 1 byte
  uint8_t maxAlowedRules; // 1 byte
  uint8_t maxAlowedHumid; // 1 byte
  uint16_t srto; //System Reset Time Out
  boolean is_After_reset;
  boolean sdCardPresent; // 1 byte
  boolean loggerEnable; //1 byte

} sys; //13 bytes

struct netConfiguration {
  //  uint8_t mac[6]; //6 bytes
  int webPort;
  int snmpPort;
  uint32_t NTPSyncInterval;
  int timeZone; // 1 byte
  boolean dhcp;
  IPAddress ip; //6 bytes
  IPAddress mask; //6 bytes
  IPAddress gateway; //6 bytes
  IPAddress timeServer; //6 bytes
  IPAddress dns; //6 bytes
  char readCommunity[MAX_COMMUNITY_DESCRIPTION];
  char writeCommunity[MAX_COMMUNITY_DESCRIPTION];
//  char user64[MAX_64];
//  char admin64[MAX_64];
  char usernameU[MAX_64];
  char passwordU[MAX_64];
  char usernameA[MAX_64];
  char passwordA[MAX_64];
  char hostName[MAX_HOST_DESCRIPTION]; //16 bytes

} netConf; //63bytes

void loadSys() {
  sys.mac[0] = 0xAE;   sys.mac[1] = 0xAD;   sys.mac[2] = 0xBE;   sys.mac[3] = 0xEF;   sys.mac[4] = 0xFE;   sys.mac[5] = 0xED; //arduino example
  // sys.mac[0] = 0x00;   sys.mac[1] = 0x50;   sys.mac[2] = 0xC2;   sys.mac[3] = 0x00;   sys.mac[4] = 0x00;   sys.mac[5] = 0x20; //dvtel prefix
  sys.maxAlowedAnalog = 9;
  sys.maxAlowedDigital = 9;
  sys.maxAlowedOneWire = 9;
  sys.maxAlowedRelay = 9;
  sys.maxAlowedRules = 15;
  sys.srto = 720; //5 minutes
  sys.sdCardPresent = false;
  sys.loggerEnable = true;
  sys.is_After_reset=false;
}

void loadNet() { //factory defaults
  netConf.ip[0] = 192; netConf.ip[1] = 168; netConf.ip[2] = 169; netConf.ip[3] = 10; //static ip 192.168.169.10
  netConf.mask[0] = 255;  netConf.mask[1] = 255;  netConf.mask[2] = 255;  netConf.mask[3] = 0; //subnet mask 255.255.255.0
  netConf.gateway[0] = 192; netConf.gateway[1] = 168; netConf.gateway[2] = 169; netConf.gateway[3] = 1; //gateway 192.168.169.1
  netConf.dns[0] = 192; netConf.dns[1] = 168; netConf.dns[2] = 169; netConf.dns[3] = 1; //dns 192.168.169.1
  netConf.timeServer[0] = 132;   netConf.timeServer[1] = 163;   netConf.timeServer[2] = 4; netConf.timeServer[3] = 102 ;// 132, 163, 4, 102
  netConf.NTPSyncInterval = 30; // [minutes]
  netConf.timeZone = 2; // Central European Time
  netConf.dhcp = true;
  netConf.webPort = 81;
  netConf.snmpPort = 161;

  sprintf(netConf.hostName, "BS-Sys"); //this line to replace dhcp.h line 47
  sprintf(netConf.readCommunity, "public");
  sprintf(netConf.writeCommunity, "private");

  sprintf(netConf.usernameU, "user");
  sprintf(netConf.passwordU, "user");
  sprintf(netConf.usernameA, "admin");
  sprintf(netConf.passwordA, "admin");

//  sprintf(netConf.user64, "dXNlcjp1c2Vy");
//  sprintf(netConf.admin64, "YWRtaW46YWRtaW4=");
}
#endif
