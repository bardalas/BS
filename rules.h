#ifndef RULES
#define RULES

#define DISABLE              127
#define ALWAYS_ON            true
#define MAX_RULE_DESCRIPTION 10

#define START    0
#define END      1
#define CLOCK    2

TimeElements startElementsTmp;
TimeElements endElementsTmp;

struct ruleConfiguration {
  uint8_t state[MAX_RULES];
  boolean trig[MAX_RULES]; //indicates if the rule is activated
  boolean analogTrig[MAX_RULES];
  boolean oneWireTrig[MAX_RULES];
  boolean weekDay[MAX_RULES][7];
  // boolean isFree[MAX_RULES];
  uint8_t relayAction[MAX_RULES];
  uint8_t digitalState[MAX_RULES]; //1=high ,0=low, 127=NA
  uint8_t relay[MAX_RULES]; // the relay to State once the rule is triggered
  uint8_t analog[MAX_RULES];//the analog input for the rule to treat // DISABLE (127) if not set
  uint8_t humid[MAX_RULES];//the analog input for the rule to treat // DISABLE (127) if not set
  uint8_t digital[MAX_RULES];//the digital input for the rule to treat //  DISABLE (127) if not set
  uint8_t oneWire[MAX_RULES];//the oneWire address or input  for the rule to treat // DISABLE (127) if not set
  uint32_t digitalCount[MAX_RULES]; //max
  
  float analogMax[MAX_RULES];//max Analog value to trigg the rule
  float analogMin[MAX_RULES]; //min Analog value to trigg the rule
  float analogHister[MAX_RULES];
  
//  double humidMax[MAX_RULES];//max Analog value to trigg the rule
//  double humidMin[MAX_RULES]; //min Analog value to trigg the rule
//  double humidHister[MAX_RULES];  
  
  float oneWireMax[MAX_RULES]; //Max One Oire valude to trigg the rule
  float oneWireMin[MAX_RULES]; //min One Wire value to trigg the rule
  float oneWireHister[MAX_RULES];

  time_t startTime[MAX_RULES];
  time_t endTime[MAX_RULES];
 // uint8_t rulesCount;

  char description[MAX_RULES][MAX_RULE_DESCRIPTION];
}
rule;

boolean activeBySchedule(uint8_t i);
boolean checkRepeat(uint8_t i);
boolean activeByDay(uint8_t i);
boolean activeByAnalog(uint8_t i);
boolean activeByOneWire(uint8_t i);
boolean activeByDigital(uint8_t i);
boolean activeByCounter(uint8_t i);
boolean isSchedule(uint8_t i);
void ruleActivate(uint8_t i);
void ruleDeActivate(uint8_t i);
void printRule(uint8_t i);
void clearTimeElements(TimeElements* te);
void clearDays(uint8_t i);
void setStartTime(char* time, uint8_t i);
void clearRuleSettings(uint8_t i);
void clearRule(uint8_t i);
void setRuleOffByRelay(uint8_t i);
void removeRule(uint8_t i);
void addRule(uint8_t i);
void setStartEnd(uint8_t i);


//This function is called on Factory reset only
void initRules() {
  for (int i = 0; i < MAX_RULES; i++) {
    clearRule(i);
  }
//  rule.rulesCount = 0;
}

void applyRules() {
  for (uint8_t i = 0; i < sys.maxAlowedRules; i++) {
    //   Serial.print(i);Serial.print("rule.state:");Serial.println(rule.state[i]);
    if (!rule.state[i] == 0) { //check if the rule is off or deleted, nothing to check.. leave the relay as is (SNMP might override)

      //     Serial.print(i); Serial.print(" : activeBySchedule "); Serial.println(activeBySchedule(i));
      //      Serial.print(i); Serial.print(" : Schedule "); Serial.println(checkRepeat(i));
      //verify that we have at least 1 trigger to check, otherwise - deActive the rule and get proceed to next rule !
      if ( (rule.analog[i] != NA) || (rule.oneWire[i] != NA) || (rule.digital[i] != NA) || isSchedule(i) )
      {
        //at least one trigger is defined
        //check who is triggered (if not active then it should return us "true")
/*
        Serial.println(activeByAnalog(i) ? "an on" : "an off");
        Serial.println(activeByOneWire(i) ? "ow on" : "ow off");
        Serial.println(activeByDigital(i) ? "dg on" : "dg off");
        Serial.println(activeByCounter(i) ? "count on" : "count off");
        Serial.println(activeBySchedule(i) ? "sched on" : "sched off");
*/
        if ( activeByAnalog(i) && activeByOneWire(i) && activeByDigital(i) && activeByCounter(i) && activeBySchedule(i) ) {
          //all defined triggers are on
          rule.trig[i] = true;
          ruleActivate(i);
        }
        else { //NOT all defined triggers are on (at least one is not)
          ruleDeActivate(i);
          rule.trig[i] = false;
        }
      }
      else {//no triggers defined
        ruleDeActivate(i);
        rule.trig[i] = false;
        //        Serial.print(i); Serial.println(" : rule is On");
      }
    }
    else { //Rule is off
      //    ruleDeActivate(i);
      rule.trig[i] = false;
      //      Serial.print(i); Serial.println(" : rule is Off");
    }
  }//go on with next rule
}

boolean isSchedule(uint8_t i) {
  //verify that both start and end is set
  if (rule.startTime[i] != 0 && rule.endTime[i] != 0)
    return true;
  return false;
}

boolean activeBySchedule(uint8_t i) {
  //verify that both start and end are set
  //  Serial.println();
  /*
    Serial.print("start:"); Serial.println(rule.startTime[i]);
    Serial.print("now: "); Serial.println(now());
    Serial.print("end: "); Serial.println(rule.endTime[i]);
  */
  if (rule.startTime[i] == 0 || rule.endTime[i] == 0)
    return true; //logic 1 will ignor us

  if (now() < rule.startTime[i]) return false; // now < start
  else if (now() > rule.startTime[i] && now() < rule.endTime[i]) return activeByDay(i); //schedule is on, check the day //start < now < end
  else if (now() > rule.endTime[i]) return false; //end < now
  else return false;
}

//return true if there is at least 1 day set to repeate
//rutern false if none //equivalent to NA
boolean checkRepeat(uint8_t i) {
  for (uint8_t j = 0; j < 7; j++) {
    if (rule.weekDay[i][j]) {
      return true;
    }
  }
  return false;
}

boolean activeByDay(uint8_t i) {
  // Serial.print("found a repeat ? : "); Serial.println(checkRepeat(i));
  if (!checkRepeat(i)) //is there any repeat day ??
    return true; //the days are ignored - return 1 so the logical "&" operator will ignor it

  boolean active = false;
  for (uint8_t j = 0 ; j < 7; j++) { //go over all days
    if (rule.weekDay[i][j]) {// search a setted day
      //found a day that is setted on the "j" spot
      if (weekday() == (j + 1)) { //is it today ? (j starts with 0-Sunday, weekday 1-sunday so we need to add 1 to j before compare
        //       Serial.print("Today is the day !!"); Serial.print(j);
        active = true;
      }
    }
  }
  return active;
}

boolean activeByAnalog(uint8_t i) {
  if (rule.analog[i] == NA)
    return true; //the sensor is ignored - return 1 so the logical "&" operator will ignor it

  //first thing to check is - are we out off limits ??
  //no way we turn off if out of limits
  if ( (analog.Value[rule.analog[i]] > rule.analogMax[i]) || (analog.Value[rule.analog[i]] < rule.analogMin[i]) ) {
    rule.analogTrig[i] = true;
    return true; //values out of limits ! - turn on
  }
  //if we are out of limits we will get here and not check this logic
  //turn off only if we are in limits reducing the hysteresis value
  else if ( (analog.Value[rule.analog[i]] > (rule.analogMin[i] + rule.analogHister[i]) ) &&
            (analog.Value[rule.analog[i]] < rule.analogMax[i] - rule.analogHister[i]) &&
            rule.analogTrig[i] )
  {
    rule.analogTrig[i] = false;
    return false; // values return to min/max limits with hysteresis consideration - turned off
  }

  //not action item found
  //stay as you are
  else
  {
    return rule.analogTrig[i];
  }
}

boolean activeByOneWire(uint8_t i) {
  if (rule.oneWire[i] == NA)
    return true; //the sensor is ignored - return 1 so the logical "&" operator will ignor it

  //first thing to check is - are we out off limits ??
  //no way we turn off if out of limits
  if ( (oneWire.ValueTemp[rule.oneWire[i]] > rule.oneWireMax[i]) || (oneWire.ValueTemp[rule.oneWire[i]] < rule.oneWireMin[i]) ) {
    rule.oneWireTrig[i] = true;
    return true; //values out of limits ! - turn on
  }
  //if we are out of limits we will get here and not check this logic
  //turn off only if we are in limits reducing the hysteresis value
  else if ( (oneWire.ValueTemp[rule.oneWire[i]] > (rule.oneWireMin[i] + rule.oneWireHister[i]) ) &&
            (oneWire.ValueTemp[rule.oneWire[i]] < rule.oneWireMax[i] - rule.oneWireHister[i]) &&
            rule.oneWireTrig[i] )
  {
    rule.oneWireTrig[i] = false;
    return false; // values return to min/max limits with hysteresis consideration - turned off
  }

  //not action item found
  //stay as you are
  else
  {
    return rule.oneWireTrig[i];
  }
}

boolean activeByDigital(uint8_t i) {
  if (rule.digital[i] == NA || rule.digitalState[i] == NA) //no digital set or no state set
    return true; //the sensor is ignored - return 1 so the logical & will ignor it
  return (rule.digitalState[i] == digital.State[rule.digital[i]]);
}

boolean activeByCounter(uint8_t i) {
  //  Serial.println("activeByCounter:");
  if (rule.digital[i] == NA)
    return true; //the sensor is ignored - return 1 so the logical & will ignor it
  //  Serial.println(digital.Counter[rule.digital[i]]);
  //  Serial.println(rule.digitalCount[i]);
  return (digital.Counter[rule.digital[i]] >= rule.digitalCount[i]);
}

void clearTimeElements(TimeElements *te) {
  te->Second = 0;
  te->Minute = 0;
  te->Hour = 0;
  te->Wday = 0; // day of week, sunday is day 1
  te->Day = 0;
  te->Month = 0;
  te->Year = 0;
  //  Serial.print("te in:  "); Serial.println(te->Year);
  //  Serial.print("Year in:  "); Serial.println(startElementsTmp.Year);
}

void clearDays(uint8_t i) {
  for (uint8_t j = 0; j < 7; j++)
    rule.weekDay[i][j] = false;
}


void ruleActivate(uint8_t i) {
  // Serial.print("rule activate rule.relay[i]: "); Serial.println(rule.relay[i]);
  if (rule.relay[i] != NA) {
    //    Serial.println("rule.relay[i] != NA");
    if (rule.relayAction[i] == OFF) { //OFF (set low to the pin)
      setRelayOff(rule.relay[i]); //we activate the rule so do as it says !!
      //      Serial.println("setRelayOff");
    }
    else if (rule.relayAction[i] == ON) { //ON(set high to the pin)
      setRelayOn(rule.relay[i]);
      //      Serial.println("setRelayOn");
    }
    else {
      //     Serial.println("No Action Selected");
      //no settings to relay found - this state is not possible sinse we do not allow it in web, but hey go figure..
    }
    //    else if {} //no relay action requiered, maybe we need to send mail ??
  }
  // else Serial.println("Rule active - relay NA");
  return;
}

void ruleDeActivate(uint8_t i) {
  if (rule.relay[i] != NA) {
    //   Serial.println("rule.relay[i] != NA");
    if (rule.relayAction[i] == OFF) { //OFF (set low to the pin)
      setRelayOn(rule.relay[i]); //we de-activate the rule so do the opposite
      //     Serial.println("setRelayOn");
    }
    else if (rule.relayAction[i] == ON) { //ON(set high to the pin)
      setRelayOff(rule.relay[i]);
      //      Serial.println("setRelayOff");
    }
  }
  else  //no relay assigned to the rule - do nothing (maybe mail or other action in the future need to be sut off)
    return;
}

void parseTime(char* time, uint8_t se) {
  if (time[0] == 0) { //check if we got something to set
    //    Serial.print("Zero time to set");
    return; //get out - don't put junk !!
  }
  char year[5];  char month[3];  char day[3];  char hour[3];  char minute[3];

  year  [0] = time[0];   year  [1] = time[1];  year[2] = time[2];  year[3] = time[3]; year[4] = '\0';
  month [0] = time[5];   month [1] = time[6];  month[2] = '\0';
  day   [0] = time[8];   day   [1] = time[9];  day[2] = '\0';
  hour  [0] = time[11];  hour  [1] = time[12]; hour[2] = '\0';
  minute[0] = time[14];  minute[1] = time[15]; minute[2] = '\0';

  switch (se) {
    case START: {
     //   Serial.println(year);
        startElementsTmp.Year = (atoi(year) - 1970);
        startElementsTmp.Month = atoi(month);
        startElementsTmp.Day = atoi(day);
        startElementsTmp.Hour = atoi(hour);
        startElementsTmp.Minute = atoi(minute);
        break;
      }
    case END: {
        endElementsTmp.Year = (atoi(year) - 1970);
        endElementsTmp.Month = atoi(month);
        endElementsTmp.Day = atoi(day);
        endElementsTmp.Hour = atoi(hour);
        endElementsTmp.Minute = atoi(minute);
        break;
      }

    case CLOCK: {
  //            Serial.print("Time: "); Serial.println(time);
        if (!(atoi(year) == 0 && atoi(month) == 0 && atoi(day) == 0 && atoi(hour) == 0 && atoi(minute) == 0 )) {
          setTime(atoi(hour), atoi(minute), 00, atoi(day), atoi(month), atoi(year));
        }
      }
    default:;
  }
}

void setStartEnd(uint8_t i) {
  time_t start, end;
  start = makeTime(startElementsTmp);
  // Serial.println(start);
  end = makeTime(endElementsTmp);
  // Serial.print("Start:"); Serial.println(start);
  //  Serial.print("End:"); Serial.println(end);
  if (start == 4294880896 && end == 4294880896) { //4294880896 is what we get if not a valid input
    rule.startTime[i] = 0;
    rule.endTime[i] = 0;
  }
  else  if (start < end) {
    rule.startTime[i] = start;
    rule.endTime[i] = end;
  }
}
/*

void addRule() {
  int i = 0;
  while (i < sys.maxAlowedRules) {
    if (rule.isFree[i]) {
      clearRule(i);
      rule.rulesCount++;
      rule.isFree[i] = false;
      break;
    }
    i++;
  }
}

void removeRule(uint8_t i) {
  ruleDeActivate(i);
  rule.rulesCount--;
  rule.isFree[i] = true;
}
*/

void clearRule(uint8_t i) {
  rule.state[i] = false; //on or off
  rule.relay[i] = NA; //The relay that is assigned to the rule
  rule.trig[i] = false; //Flag to indicate if the rule is activated and relay/mail has been sent is triged
  rule.analogTrig[i] = false;
  rule.oneWireTrig[i] = false;
  rule.analog[i] = NA; //The analog input that is assigned to the rule
  rule.digital[i] = NA; //The digital input that is assigned to the rule
  rule.digitalState[i] = NA;
  rule.oneWire[i] = NA; //the oneWire adress or pointer to get the data from
  rule.analogMin[i] = 0; //max Analog value to trigg the rule
  rule.analogMax[i] = 0; //min Analog value to trigg the rule
  rule.analogHister[i] = 0; //hysteresis

//  rule.humidMin[i] = 0; //max Humidity value to trigg the rule
//  rule.humidMax[i] = 0; //min Humidity value to trigg the rule
//  rule.humidHister[i] = 0; //hysteresis
  
  rule.oneWireMax[i] = 0; //Max One Oire valude to trigg the rule
  rule.oneWireMin[i] = 0; //min One Wire value to trigg the rule
  rule.oneWireHister[i] = 0; //hysteresis

  rule.startTime[i] = 0;
  rule.endTime[i] = 0;

  // rule.isFree[i] = true; //set this rule as free and ready to go
  rule.relayAction[i] = NA; //what to do with relay when rule is trigged ?

  clearDays(i);
  //  clearTimeElements(rule.stElements[i]);
  //  clearTimeElements(rule.etElements[i]);
  sprintf(rule.description[i], "Empty Rule %d", i );
}

void setRuleOffByRelay(uint8_t i) {
  //Check if there is a rule that uses this relay and overRide it
  //If a rule is using this relay then turn off the rule and overRide the relay state
  for (uint8_t j = 0; j < sys.maxAlowedRules; j++) {
    if (rule.relay[j] == i) { //check if rule# j uses relay number i
      rule.state[j] = false; //turn off the rule
    }
  }
}
#endif
