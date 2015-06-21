#ifndef SNMP
#define SNMP

//----------------------------------- SNMP ----------------------------------------//


#define ANALOG_START_OID 37 //37-45
#define DIGITAL_START_OID 70 //70-78
#define COUNTER_START_OID 10 //10-18
#define ONEWIRE_START_OID 49 //49-57
#define RELAY_START_OID 19 //19-35 only odd numbers 19,21,23,25,27,29,31,33,35
#define HUMIDITY_START_OID // 58-66
#define PULSE_START_OID  20//20 only pair numbers 20,22,24,26,28,30,32,34,36
#define PWM_START_OID 46 //46-48

#define IO 1
#define NETWORK 2
#define UNKNOWN_TYPE 127

char oidPrefix [SNMP_MAX_OID_LEN];
char oid        [SNMP_MAX_OID_LEN];
uint8_t oidIONumber;
uint8_t oidType;
uint8_t sensorType;
uint8_t sensorIndex;
//EthernetUDP SNMP_UDP;
SNMP_API_STAT_CODES api_status;
//SNMP_ERR_CODES status;

void parseOid();
void clearRule(uint8_t i);


// ===========================================================================
//                 SNMP Initialization
// ===========================================================================
void initSNMP() {
  // SNMP UDP needs to be open all the time so we won't miss incomming packets
// we can NOT open it once every main loop --> check for income --> close
  SNMP_UDP.begin(netConf.snmpPort); //SNMP
  
  api_status = SerialSNMP.begin(netConf.readCommunity, netConf.writeCommunity); //read write
  if (!api_status == SNMP_API_STAT_SUCCESS)
  {
//    Serial.println("SNMP Community Fail");//Send serial output for failure
    return;
  }
  sprintf(oidPrefix,   "1.3.6.1.4.1.45624.3");
}


// ===========================================================================
//                 SNMP incomming packets listen & handler
// ===========================================================================
void listenSNMP() {

  int packetSize = SNMP_UDP.parsePacket();
  if (packetSize > 0)
  {
    resetSysTimout();
    SNMP_UDP.read(SerialSNMP._packet, packetSize);

    SNMP_PDU pdu;
    SNMP_API_STAT_CODES api_status = SerialSNMP.requestPdu(&pdu);

    if (api_status == SNMP_API_STAT_SUCCESS) { //check Community values (this is sort of a username and password..)
      pdu.OID.toString(oid);
      Serial.println(oid);
      parseOid();
      pdu.VALUE.encode(SNMP_SYNTAX_NULL, NULL);
      switch (oidType) {
        case IO: {
            pdu.error = SNMP_ERR_NO_ERROR;
            pdu.VALUE.syntax = SNMP_SYNTAX_INT;
            pdu.VALUE.size = 2;

            if (sensorType == ANALOG) {
              if      (pdu.type == SNMP_PDU_GET)      pdu.VALUE.encode32(SNMP_SYNTAX_INT32, (int32_t)((analog.Value[sensorIndex]) * 100) );
              else if (pdu.type == SNMP_PDU_SET)      pdu.error = SNMP_ERR_READ_ONLY;
              else                                    pdu.error = SNMP_ERR_GEN_ERROR;
            }

            else if (sensorType == COUNTER) {
              if      (pdu.type == SNMP_PDU_GET)       pdu.VALUE.encode(SNMP_SYNTAX_INT, (int)(digital.Counter[sensorIndex]) );
              else if (pdu.type == SNMP_PDU_SET)       pdu.error = SNMP_ERR_READ_ONLY;
              else                                     pdu.error = SNMP_ERR_GEN_ERROR;
            }

            else if (sensorType == DIGITAL) {
              if      (pdu.type == SNMP_PDU_GET)       pdu.VALUE.encode(SNMP_SYNTAX_INT, (int)(digital.State[sensorIndex]) );
              else if (pdu.type == SNMP_PDU_SET)       pdu.error = SNMP_ERR_READ_ONLY;
              else                                     pdu.error = SNMP_ERR_GEN_ERROR;
            }

            else if ( sensorType == ONEWIRE && (!oneWire.isFree[sensorIndex])) {
              if      (pdu.type == SNMP_PDU_GET)       pdu.VALUE.encode(SNMP_SYNTAX_INT, (int32_t)((oneWire.ValueTemp[sensorIndex]) * 100) );
              else if (pdu.type == SNMP_PDU_SET)       pdu.error = SNMP_ERR_READ_ONLY;
              else                                     pdu.error = SNMP_ERR_GEN_ERROR;
            }

            else if ( sensorType == RELAY ) {
              if      (pdu.type == SNMP_PDU_GET)       pdu.VALUE.encode(SNMP_SYNTAX_INT, (int)(relay.State[sensorIndex]) );
              else if (pdu.type == SNMP_PDU_SET) {
                int16_t state;
                if (pdu.VALUE.decode(&state) == SNMP_ERR_NO_ERROR) {
                  setRuleOffByRelay(sensorIndex); //Turn off any rule that this relay is related to
                  setRealyState(sensorIndex, state); //set the new relay state as got from SNMP
                }
                else pdu.error = SNMP_ERR_GEN_ERROR;
              }
              else                                     pdu.error = SNMP_ERR_GEN_ERROR;
            }

            break;
          }
        case NETWORK: {
            //fill here network OID handler
            break;
          }
        case UNKNOWN_TYPE : {
            pdu.VALUE.encode(SNMP_SYNTAX_NULL, NULL);
            pdu.error = SNMP_ERR_NO_SUCH_NAME;
            break;
          }
        default:          break;
      }
    }

    else {
      pdu.error = SNMP_ERR_AUTHORIZATION_ERROR;
      Serial.print(api_status);  Serial.println(": SNMP_COMMUNITY_FAIL");
    }

    //prepare response
    pdu.type = SNMP_PDU_RESPONSE;
    SerialSNMP.responsePdu(&pdu);
    //   pdu.type = SNMP_PDU_RESPONSE;
    //send respons
    if (SNMP_UDP.beginPacket(SNMP_UDP.remoteIP(), SNMP_UDP.remotePort()) == 1)
    {
      for (uint8_t i = 0; i < SerialSNMP._packetPos; i++)
      {
        SNMP_UDP.write((char) SerialSNMP._packet[i]);
      }

      if (SNMP_UDP.endPacket() == 0)
      {
        //socket wrtoe error
      }
    }
    else
    {
      //socket open error
    }

    //    SerialSNMP.freePdu(&pdu);

  }
//  ShowSockStatus();
//  SNMP_UDP.stop();
//  Serial.println("stop snmp");
//  ShowSockStatus();
}

void parseOid() {
  // Serial.println(oid);
  sensorType = UNKNOWN_TYPE;
  sensorIndex = NA;

  uint8_t oidLength = strlen(oid);
  if ( (oid[oidLength - 1] - '0') != 0) {
    //last digit must be 0 !!
    oidType = UNKNOWN_TYPE;
    return;
  }

  if ( strlen(oid) ==  (strlen(oidPrefix) + 5)) {
    oidType = IO; //other commands has different lengths !
    //extract the oidNumber (sensor id)
    char tmp[2];
    tmp[0] = oid[(strlen(oidPrefix) + 1)];
    tmp[1] = oid[(strlen(oidPrefix) + 2)];
    oidIONumber = atoi(tmp);
    if (oidIONumber >= ANALOG_START_OID && oidIONumber < (ANALOG_START_OID + sys.maxAlowedAnalog) ) {
      //      Serial.println("Analog");
      sensorType = ANALOG;
      sensorIndex = oidIONumber - ANALOG_START_OID;
    }
    else if (oidIONumber >= DIGITAL_START_OID && oidIONumber < (DIGITAL_START_OID + sys.maxAlowedDigital) ) {
      //      Serial.println("Digital");
      sensorType = DIGITAL;
      sensorIndex = oidIONumber - DIGITAL_START_OID;
    }
    else if (oidIONumber >= COUNTER_START_OID && oidIONumber < (COUNTER_START_OID + sys.maxAlowedDigital)) {
      //      Serial.println("Counter");
      sensorType = COUNTER;
      sensorIndex = oidIONumber - COUNTER_START_OID;
    }
    else if (oidIONumber >= ONEWIRE_START_OID && oidIONumber < (ONEWIRE_START_OID + sys.maxAlowedOneWire)) {
      //     Serial.println("OneWire");
      sensorType = ONEWIRE;
      sensorIndex = oidIONumber - ONEWIRE_START_OID;
    }
    else if (oidIONumber >= RELAY_START_OID && oidIONumber <= (RELAY_START_OID + (sys.maxAlowedRelay * 2))) {
      //      Serial.println("Relay");
      sensorType = RELAY;
      if ( (oidIONumber % 2) != 0) { //odd numbers only
        sensorIndex = (oidIONumber - RELAY_START_OID) / 2;
      }
      else { //found odd number
        //fill here Relay Pulser handler
        oidType = UNKNOWN_TYPE;
        //       Serial.print("unknown Type");
      }
    }
    else {
      oidType = UNKNOWN_TYPE;
    }
    //DO NOT DELETE - THIS IS READY FOR OTHER SENSORS
    // else if (oidIONumber>=HUMIDITY_START_OID && oidIONumber<=(HUMIDITY_START_OID+sys.maxAlowedHumidity)) sensorType=HUMIDITY;
    // else if (oidIONumber>=PULSE_START_OID && oidIONumber<=(PULSE_START_OID+sys.maxAlowedPulse)) sensorType=PULSE;
    // else if (oidIONumber>=PWM_START_OID && oidIONumber<=(PWM_START_OID+sys.maxAlowedPwm)) sensorType=PWM;

  }
  //else if {}

  else { //oid length does not mach to sensors oid length
    oidType = UNKNOWN_TYPE;
  }
}
#endif
