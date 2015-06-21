#ifndef ANALOG
#define ANALOG

//analog configuration and data
#define MAX_ANALOG_DESCRIPTION 10 //16+1 for '\0'
#define ANALOG_START_PIN 54

struct analogConfiguration {
  uint16_t Min[MAX_ANALOG];
  uint16_t Max[MAX_ANALOG];
//  uint16_t Mul[MAX_ANALOG];
//  uint16_t Div[MAX_ANALOG];
  uint8_t logInterval[MAX_ANALOG];
  uint8_t logMinuteTick[MAX_ANALOG];
  uint8_t minVoltage[MAX_ANALOG];

  float Offset[MAX_ANALOG];
  float m[MAX_ANALOG];
  float b[MAX_ANALOG];

  float Value[MAX_ANALOG]; 

  char Description[MAX_ANALOG][MAX_ANALOG_DESCRIPTION];
}
analog;

void initAnalog() {
  for (int i = 0; i < MAX_ANALOG; i++) {
//    analog.Mul[i] = 1;
//    analog.Div[i] = 1;
    analog.Offset[i] = 0;
    analog.b[i] = 0;
    analog.m[i] = 1;
    analog.minVoltage[i] = 0;
    analog.Max[i] = 1023;
    analog.Min[i] = 0;
    //   analog.Data[i] = 0;
    analog.Value[i] = 0;
    sprintf(analog.Description[i], "Analog %d", i );
    analog.logInterval[i] = 0;
    analog.logMinuteTick[i] = 0;
  }
}

void updateAnalog() {
  for ( int i = 0; i < sys.maxAlowedAnalog; i++) {
    /*
      analog.Data[i] = analogRead(ANALOG_START_PIN + i);
      analog.Value[i] = (float) ( ( ( analog.Data[i] ) * analog.Mul[i]) / analog.Div[i] ) + analog.Offset[i];*/



    float Val = analogRead(ANALOG_START_PIN + i) ;
    //   analog.Value[i] = (float) ( ( ( Val ) * analog.Mul[i]) / analog.Div[i] ) + analog.Offset[i];
    analog.Value[i] = Val * analog.m[i] + analog.b[i] + analog.Offset[i];
  }
}

void calcM(int i) {
  if (analog.minVoltage[i]==0)
  analog.m[i] = (((float)analog.Max[i] - (float)analog.Min[i] + 1 ) / 1024); // 0-5 volt
  else
  analog.m[i] = (((float)analog.Max[i] - (float)analog.Min[i] + 1 ) / 819); // 1-5 volt  (max_adc - min_adc) (1024-215)
 // Serial.print("analog.m[i]:"); Serial.println(analog.m[i]);
}

void calcB(int i) {
  analog.b[i] = (float)analog.Max[i] - (1024 * analog.m[i]) ;
//  Serial.print("analog.b[i]:"); Serial.println(analog.b[i]);
}


int getAnalogInt(uint8_t i) {
  if (i < 0 || i >= sys.maxAlowedAnalog) return -1;
  return (int)(analog.Value[i] * 10);
}

float getAnalogFloat(uint8_t i) {
  if (i < 0 || i >= sys.maxAlowedAnalog) return -1;
  return analog.Value[i];
}
#endif
