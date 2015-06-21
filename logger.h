#ifndef LOGGER
#define LOGGER

#define ANALOG   0
#define RULES    1
#define ONEWIRE  2
#define HUMID    3
#define DIGITAL  4
#define RELAY    5
#define NET      6
#define SYS      7
#define COUNTER  8

//#include <string.h>
#include <stdio.h>

void initLogSensor();
void logSensor(uint8_t type, uint8_t i);
void writeToFile(char *path, char *fileName, char *description, char *value, uint8_t id);
void sdFail();
//
char path[30];
char description[16];
char fileName[30];
char value[20];
char tmp[20];
File logfile;

struct loggerConfiguration {
  boolean analog[MAX_ANALOG];
#ifdef DHT
  boolean humid[MAX_HUMID];
#endif
  boolean oneWire[MAX_ONE_WIRE];
  boolean digital[MAX_DIGITAL];
  boolean toSave;
}
logger;


void initLogSensor(){
  for (uint8_t i = 0; i < MAX_ANALOG; i++) {
    logger.analog[i] = false;
  }
#ifdef DHT
  for (uint8_t i = 0; i < MAX_HUMID; i++) {
    logger.humid[i] = false;
  }  
#endif
  for (uint8_t i = 0; i < MAX_ONE_WIRE; i++) {
    logger.oneWire[i] = false;
  }
  for (uint8_t i = 0; i < MAX_DIGITAL; i++) {
    logger.digital[i] = false;
  }
  logger.toSave=false;
  analogSaveIndex=0;
#ifdef DHT
  humidSaveIndex=0;
#endif
  digitalSaveIndex=0;
  oneWireSaveIndex=0;
}

boolean initSD() {

  //  Serial.print("SD card: ");
  digitalWrite(ETH_CS, HIGH); //disable Ethernet CS
  delay(1);
  if (!SD.begin(SD_CS)) {
    //    Serial.println("fail");
    sys.sdCardPresent = false;
    return false;
  }
  //  Serial.println("ok");
  sys.sdCardPresent = true;
  return true;
}

void resetLogTicks() {
  for (uint8_t i = 0; i < sys.maxAlowedAnalog;  i++) analog.logMinuteTick[i]  = 0;
#ifdef DHT
  for (uint8_t i = 0; i < sys.maxAlowedHumid;   i++) humid.logMinuteTick[i]   = 0;
#endif
  for (uint8_t i = 0; i < sys.maxAlowedDigital; i++) digital.logMinuteTick[i] = 0;
  for (uint8_t i = 0; i < sys.maxAlowedOneWire; i++) oneWire.logMinuteTick[i] = 0;
}

//Checks if we need to log according to log interval of each sensor and if the sensor is active
void checkLogTicks() {
  if (!sys.loggerEnable) { // check if logger is enabled by the system
    //    Serial.println("Logging is not Alowed");
    return;
  }

  //check which sensor is due (in minutes)
  for (uint8_t i = 0; i < sys.maxAlowedAnalog; i++) { //check analog
    if (analog.logInterval[i] != 0) { //should we save ??
      analog.logMinuteTick[i]++; // a minute has past from last check, progress the counter
      if (analog.logInterval[i] == analog.logMinuteTick[i]) { //is it time to save ?
        logger.analog[i]=true;
        logger.toSave=true;
    //    logSensor(ANALOG, i);
        analog.logMinuteTick[i] = 0; //done saving, reset the save indicator
      }
    }//no need to save this sensor
  }//keep checking next sensor
  
#ifdef DHT
  //check which sensor is due (in minutes)
  for (uint8_t i = 0; i < sys.maxAlowedHumid; i++) { //check analog
    if (humid.logInterval[i] != 0) { //should we save ??
      humid.logMinuteTick[i]++; // a minute has past from last check, progress the counter
      if (humid.logInterval[i] == humid.logMinuteTick[i]) { //is it time to save ?
        logger.humid[i]=true;
        logger.toSave=true;
    //    logSensor(ANALOG, i);
        humid.logMinuteTick[i] = 0; //done saving, reset the save indicator
      }
    }//no need to save this sensor
  }//keep checking next sensor
#endif

  for (uint8_t i = 0; i < sys.maxAlowedDigital; i++) { //check Digital
    if (digital.logInterval[i] != 0) { //should we save ??
      digital.logMinuteTick[i]++; // a minute has past from last check, progress the counter
      if (digital.logInterval[i] == digital.logMinuteTick[i]) { //is it time to save ?
        logger.digital[i]=true;
        Serial.print("mDG:"); Serial.println(i);
        logger.toSave=true;
     //   logSensor(DIGITAL, i);
        digital.logMinuteTick[i] = 0; //done saving, reset the save indicator
      }
    }//no need to save this sensor
  }//keep checking next sensor

  for (uint8_t i = 0; i < sys.maxAlowedOneWire; i++) { //check oneWire
    //   Serial.print("Checking log: "); Serial.println(i);
    if (oneWire.logInterval[i] != 0) { //should we save ??
      oneWire.logMinuteTick[i]++; // a minute has past from last check, progress the counter
      if (oneWire.logInterval[i] == oneWire.logMinuteTick[i]) { //is it time to save ?
        logger.oneWire[i]=true;
        logger.toSave=true;
 //       logSensor(ONEWIRE, i);
        oneWire.logMinuteTick[i] = 0; //done saving, reset the save indicator
      }
    }//no need to save this sensor
  }//keep checking next sensor
}

void logSensor(uint8_t type, uint8_t i) { //we got here after checking interval and a request for logging was sent
  Serial.println("logSensor");

  if (!sys.sdCardPresent) {
    Serial.println("SD Card fail trying to initSD");
    if (!initSD()) {
      Serial.println("No log");
      return; //no SD card, do not procceed with logging
    }
  }

  //SD card presented

  tmp[0] = '\0';
  path[0] = '\0';
  description[0] = '\0';
  value[0] = '\0';
  fileName[0] = '\0';

  //create the path
  strcat(path, "/Bsensors");
  switch (type) {
    case (ANALOG):  {
        strcat(path, "/Analog/");
        dtostrf(analog.Value[i], 10, 2, tmp); // put float value in C-style String
        sprintf(value, "%s", tmp); //Convert String to char[]
        //         sprintf(tmp, "%u", i);
        strcat(description, analog.Description[i]);
      } break;
#ifdef DHT
    case (HUMID):  {
        strcat(path, "/Dht/");
        char tmp2[5];
        dtostrf(humid.Humid[i], 10, 2, tmp); // put float value in C-style String
        dtostrf(humid.Temp[i], 10, 2, tmp2); // put float value in C-style String
        strcat(tmp, ",");
        strcat(tmp, tmp2);
        sprintf(value, "%s", tmp); //Convert String to char[]
        //         sprintf(tmp, "%u", i);
        strcat(description, humid.Description[i]);
      } break;
#endif   
    case (DIGITAL): {
        strcat(path, "/Digital/");
        //         sprintf(tmp, "%u", i);
        strcat(description, digital.Description[i]);
        sprintf(value, "%u", digital.Counter[i] );
      } break;
    case (ONEWIRE): {
        strcat(path, "/oneWire/");
        dtostrf(oneWire.ValueTemp[i], 10, 2, tmp); // put float value in C-style String
        sprintf(value, "%s", tmp); //Convert String to char[]
        //         sprintf(tmp, "%u", i);
        strcat(description, oneWire.Description[i]);
      } break;
    default:;
  }
  sprintf(tmp, "%u", year());
  strcat(path, tmp);
  strcat(path, "/");
  sprintf(tmp, "%u", month());
  strcat(path, tmp);
  strcat(path, "/");
  sprintf(tmp, "%u", day());
  strcat(path, tmp);
  strcat(path, "/");

  //create the file name ID.csv
  tmp[0] = '\0';
  sprintf(tmp, "%d", i); //consider remove this line, since tmp already contain the i index used for folder name
  strcat(fileName, tmp);
  strcat(fileName, ".csv");

  writeToFile(path, fileName, description, value, i);
}


void writeToFile(char *path, char *fileName, char *description, char *value, uint8_t id)
{
  // digitalWrite(10, HIGH); //disable Ethernet CS
  // see if the directory exists, create it if not.
  if ( !SD.exists(path) ) {
    //path dosn't exist, create it !
    //   Serial.print("create path: ");
    if ( SD.mkdir(path) ) Serial.println("ok");
    else {
      //could not creat path, report fail and exit
      sdFail();
      return;
    }
  }

  //  Serial.println("path exists");
  Serial.println(fileName);
  //prepare the file name (path/filename.csv)
  strcat(path, fileName);
  //check if the file exist
  Serial.println(path);
  if (! SD.exists(path)) {
    // only open a new file if it doesn't exist
    //new file, prine headers
    logfile = SD.open(path, FILE_WRITE);
    logfile.write("Date,");
    logfile.write("Time,");
    logfile.write("ID,");
    logfile.write("Description,");
    logfile.write("Value");
    logfile.write("\n");
    logfile.close();
  }

  logfile = SD.open(path, FILE_WRITE);
  // Serial.print("file write: ");
  if (logfile) {
    digitalWrite(15,HIGH);
    tmp[0] = '\0'; //clear the array
    sprintf(tmp, "%d/%d/%d,%d:%d,", day(), month(), year(), hour(), minute());
    logfile.write(tmp);
    tmp[0] = '\0'; //clear the array
    sprintf(tmp, "%d", id);
    logfile.write(tmp);
    logfile.write(",");
    logfile.write(description);
    logfile.write(",");
    logfile.write(value);
    logfile.write("\n");
    logfile.close();
    digitalWrite(15,LOW);
    Serial.println("ok");
    return;
  }
  else sdFail();
}

void sdFail() {
  Serial.println("fail");
  sys.sdCardPresent = false;
  return;
}
#endif
