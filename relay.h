#ifndef RELAY
#define RELAY

#define RELAY_START_PIN  40 //TODO: verify start pin
#define MAX_RELAY_DESCRIPTION 10

#define OFF    0 //Gnd
#define ON     1 //5V
#define NA     127

struct relayConfiguration {
  boolean State[MAX_RELAY];
  uint8_t Normaly[MAX_RELAY];
  char Description[MAX_RELAY][MAX_RELAY_DESCRIPTION];
} relay;

void initRelays();
void setRelayToNormaly(uint8_t i);
void setRelayOn(uint8_t i);
void setRelayOff(uint8_t i);
void setRealyState(uint8_t i, uint8_t state);
void toggleRelay(uint8_t i);
void setRuleOffByRelay(uint8_t i);


void initRelays() {
  for (uint8_t i = 0; i < MAX_RELAY; i++) {
    relay.State[i] = false;
    relay.Normaly[i] = OFF; //set relay to Normaly LOW (set gnd to relay pins)
    sprintf(relay.Description[i], "Relay %d", i );
  }
}

void setRelayToNormaly(uint8_t i) {
  if (relay.Normaly[i] == ON){
    digitalWrite(RELAY_START_PIN + i, HIGH);
    relay.State[i] = true;
  }
  else{
    digitalWrite(RELAY_START_PIN + i, LOW);
    relay.State[i] = false;
  }
}

void setRelayOn(uint8_t i) {
 //   Serial.println("Setting Relay On ++++++++++++++++++++++++++++++++");
 //   Serial.print(i); Serial.println(": ON");
  relay.State[i] = true;
  //  relay.Counter[i]++;
  digitalWrite(RELAY_START_PIN + i, HIGH);
}

void setRelayOff(uint8_t i) {
  //  Serial.print(i); Serial.println(": OFF");
  relay.State[i] = false;
  // relay.Counter[i]++;
 // Serial.println("Setting Relay Off ------------------------------");
 // Serial.print(i); Serial.println(": OFF");
  digitalWrite(RELAY_START_PIN + i, LOW);
}


void setRealyState(uint8_t i, uint8_t state) {
  //  Serial.print("state:"); Serial.println(state);
  if (state) setRelayOn(i);
  else setRelayOff(i);
}

void toggleRelay(uint8_t i) {
  setRuleOffByRelay(i); //override any rule that uses this relay
  digitalRead(RELAY_START_PIN + i) ?  setRelayOff(i) : setRelayOn(i);
}

uint8_t getRelay(uint8_t i) {
  if (i < 0 || i >= sys.maxAlowedRelay) return -1;
  if(relay.State[i]) return 1;
  else return 0;
}
#endif
