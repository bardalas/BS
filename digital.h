#ifndef DIGITAL
#define DIGITAL

//  digital configuration and data
#define MAX_DIGITAL_DESCRIPTION 10 //16+1 for '\0'
#define DIGITAL_START_PIN 22

struct digitalConfiguration {
  uint16_t Counter[MAX_DIGITAL];
  uint8_t logInterval[MAX_DIGITAL];
  uint8_t logMinuteTick[MAX_DIGITAL];
  boolean State[MAX_DIGITAL];
  char Description[MAX_DIGITAL][MAX_DIGITAL_DESCRIPTION];
}
digital;

void initDigital() {
  for (int i = 0; i < MAX_DIGITAL; i++) {
    digital.State[i] = true;
    digital.Counter[i] = 0; //TODO: load this value from eeprom
    digital.logInterval[i] = 0;
    digital.logMinuteTick[i] = 0;
    sprintf(digital.Description[i], "Digital %d", i );
  }
}

int getDigital(uint8_t i) {
  if (i < 0 || i >= sys.maxAlowedDigital) return -1;
  if(digital.State[i]) return 1;
  else return 0;
}

void updateDigital() {
  //digitalWrite(15,HIGH);
  //cli(); //disable interupts
  for (int i = 0; i < sys.maxAlowedDigital; i++) {
    boolean current_state = digitalRead(DIGITAL_START_PIN + i);
    digital.State[i] = current_state;
    if (last_state[i] == LOW && digitalRead(DIGITAL_START_PIN + i)) {
      digital.Counter[i]++;
    }
    last_state[i] = current_state;
  }
  //sei(); //enable interupts
  //  digitalWrite(15,LOW);
}
#endif
