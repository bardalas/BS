#ifndef HUMID
#define HUMID

//humid configuration and data
#define MAX_HUMID_DESCRIPTION 10 //16+1 for '\0'
#define HUMID_START_PIN 31

struct humidConfiguration {
  uint8_t logInterval[MAX_HUMID];
  uint8_t logMinuteTick[MAX_HUMID];
  double Humid[MAX_HUMID];
  double Temp[MAX_HUMID];
  float Offset[MAX_HUMID];
  char Description[MAX_HUMID][MAX_HUMID_DESCRIPTION];
} humid;

uint8_t humidRead;
dht DHT;

void initHumid() {
  for (int i = 0; i < MAX_HUMID; i++) {
    humid.Humid[i] = 0;
    humid.Temp[i] = 0;
    humid.logInterval[i] = 0;
    humid.Offset[i] = 0;
    humid.logMinuteTick[i] = 0;
    sprintf(humid.Description[i], "DHT %d", i );
  }
}

void updatehumid(uint8_t i) {
  if ( i < sys.maxAlowedHumid) {
    int r = DHT.read11(HUMID_START_PIN + i);
    if (r == 0) {
      humid.Humid[i] = DHT.humidity;
      humid.Temp[i] = DHT.temperature;
    }
    else {
      humid.Humid[i] = 9999;
      humid.Temp[i] = 9999;
    }
  }
}
#endif
