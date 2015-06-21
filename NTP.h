#ifndef NTP
#define NTP

/*-------- NTP code ----------*/
#define WAIT_FOR_NTP_RESPOND_SECONDS   2

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 uint8_ts of message
uint8_t packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime() {
//TODO: add check if ethernet is ready before begin UDP
//  EthernetUDP NTP_UDP;
//return(0);
 //  Serial.println(SNMP_UDP.remoteIP());
 //  Serial.println(SNMP_UDP.remotePort());
  if (NTP_UDP.begin(localPort)) {
    Serial.println("NTP_UDP_SUCCESS");
 //   ShowSockStatus(); 
    while (NTP_UDP.parsePacket() > 0) ; // UDP_FLUSH()... discard any previously received packets
//   Serial.println("Transmit NTP Request");
    
    // set all uint8_ts in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 uint8_ts of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    NTP_UDP.beginPacket(netConf.timeServer, 123); //NTP requests are to port 123
    NTP_UDP.write(packetBuffer, NTP_PACKET_SIZE);
    NTP_UDP.endPacket();
    
    uint32_t beginWait = millis();
    //wait for NTP respons
    while (millis() - beginWait < WAIT_FOR_NTP_RESPOND_SECONDS * 1000) {
      int size = NTP_UDP.parsePacket();
      if (size >= NTP_PACKET_SIZE) {
        Serial.println("Receive NTP Response");
        NTP_UDP.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
        unsigned long secsSince1900;
        // convert four uint8_ts starting at location 40 to a long integer
        secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
        secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
        secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
        secsSince1900 |= (unsigned long)packetBuffer[43];
        NTP_UDP.stop();
        return secsSince1900 - 2208988800UL + netConf.timeZone * SECS_PER_HOUR;
      }
    } //UDP connection timed out - can't sync with server
    Serial.println("No NTP Response");
    NTP_UDP.stop();
    return -1;
  }
  Serial.println("NTP_UDP_FAIL");
// ShowSockStatus(); 
  NTP_UDP.stop();
  return -1;
}
#endif
