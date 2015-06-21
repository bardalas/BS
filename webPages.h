#ifndef WEBPAGES
#define WEBPAGES
//************************       Includes             ***************************
//#include "pStrings.h"
//#include "helpFunc.h"
//************************       Definitions          ****************************

#define USER    0
#define ADMIN   1
#define FACTORY 2
#define NA      127

//boolean isAdmin = false;
uint8_t user = NA;
uint8_t j, i;
//************************Function Declerations*********************

String htmlClock();
String getClockString();

//************************Functions**************************
void initWebPages();
void oneWireP (WebServer &server, WebServer::ConnectionType type, char *, bool);
void analogP  (WebServer &server, WebServer::ConnectionType type, char *, bool);
//void humidP  (WebServer &server, WebServer::ConnectionType type, char *, bool);
void digitalP (WebServer &server, WebServer::ConnectionType type, char *, bool);
void relayP   (WebServer &server, WebServer::ConnectionType type, char *, bool);
void systemP  (WebServer &server, WebServer::ConnectionType type, char *, bool);
void rulesP   (WebServer &server, WebServer::ConnectionType type, char *, bool);
void logout   (WebServer &server, WebServer::ConnectionType type, char *, bool);


void initWebServer() {
  webserver = WebServer("", netConf.webPort);
  webserver.setDefaultCommand(&oneWireP);
  webserver.addCommand("oneWire.html", &oneWireP);
  webserver.addCommand("analog.html",  &analogP);
  //webserver.addCommand("humid.html",  &humidP);
  webserver.addCommand("digital.html", &digitalP);
  webserver.addCommand("relay.html",   &relayP);
  webserver.addCommand("rules.html",   &rulesP);
  webserver.addCommand("system.html",  &systemP);
  webserver.addCommand("logout.html",  &logout);
  //  webserver.begin();
}

bool getPrivileges(WebServer &server) {

  char user64[MAX_64];
  char admin64[MAX_64];
  char combined[17]; //8 for each field + 1 extra
  /* Serial.print("netConf.usernameU:");Serial.println(netConf.usernameU);
   Serial.print("netConf.passwordU:");Serial.println(netConf.passwordU);*/
  sprintf(combined, netConf.usernameU);
  strcat(combined, ":");
  strcat(combined, netConf.passwordU);
  /* Serial.print("user64:");Serial.println(user64);
   Serial.print("admin64:");Serial.println(admin64);
   Serial.print("combined:");Serial.println(combined);
   Serial.print("sizeof(combined):");Serial.println(sizeof(combined));*/
  uint8_t n;
  for (n = 0; n < 20; n++) {
    if (combined[n] == NULL) break;
    //   else if (combined[n]==' ') break;
  }
  // Serial.print("n:");Serial.println(n);
  base64_encode(user64, combined, n);
  sprintf(combined, netConf.usernameA);
  strcat(combined, ":");
  strcat(combined, netConf.passwordA);
  for (n = 0; n < 20; n++) {
    if (combined[n] == NULL) break;
    //   else if (combined[n]==' ') break;
  }
  base64_encode(admin64, combined, n);
  //  Serial.print("user64:");Serial.println(user64);
  if (server.checkCredentials("YnMhU3VwOlN1cCFicw==")) { //this is hard coded password for supperviser (factory) bs!Sup:Sup!bs
    user = FACTORY;
    return true;
  }

  else if (server.checkCredentials(admin64/*"YTpkbWluOmFkbWlu"*/)) { //factory default is: admin:admin
    user = ADMIN;
    return true;
  }

  else if (server.checkCredentials(user64/*"dXNlcjp1c2Vy"*/)) { //factory default is: user:user
    user = USER;
    return true;
  }
  else {
    server.httpUnauthorized();
    user = NA;
    return false;
  }
}

//one wire
void oneWireP(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  //Put this on the top of every page to prevent hack
  if (!getPrivileges(server))   return;
  server.httpSuccess();
  if (type == WebServer::GET) { //http reqested to load a page
    server.printP(DOCTYPE);
    server.printP(defaultTitle);
    server.printP(headS);
    server.printP(style); //general style
    server.printP(mmmStyle); //main menu style
    server.printP(msmowStyle); //sub menue style
    server.printP(monScript);
    server.printP(headE);
    server.printP(bodyCen);
    if (user == USER) server.printP(UmainMenu); else server.printP(AmainMenu);
    server.print(htmlClock());
    server.printP(mainMenuClose);
    server.printP(monitorSubMenu);
    if (user == USER) {}
    else {
      server.printP(slb);
    }
    P(a0) = "<div class='dispOW'>"; server.printP(a0);
    server.printP(tableHolder);
    server.printP(valTableHolder);
    P(div) = "</div>"; server.printP(div);
    server.printP(endDoc);
  }

  if (type == WebServer::POST)//client send data to server
  {
    bool repeat;
    char name[16], valueFunc[16], valueNum[16];
    repeat = server.readPOSTparam(name, 16, valueFunc, 16);
    if (strcmp(name, "func") == 0) {

      if (strcmp(valueFunc, "updateClock") == 0) { //updating the oneWireTable
        server.print(getClockString()); //sent clock string
      }

      else if (strcmp(valueFunc, "updateTable") == 0) { //updating the oneWireTable
        if (user == USER) { //user oneWire
          P(owmth) = "<table class='mot'>\n"
                     "<tr>\n"
                     "<th width='30px'>ID</th>\n"
                     "<th width='70px'>Description</th>\n"
                     "<th width='70px'>Address</th>\n";
          server.printP(owmth); //one wire table header
          for (i = 0; i < sys.maxAlowedOneWire; i++) {
            if (!oneWire.isFree[i]) {
              server.printP(trS);
              server.printP(tdS);          server.print(i);                        server.printP(tdE);
              server.printP(tdS);          server.print(oneWire.Description[i]);   server.printP(tdE);
              server.printP(tdS);
              for (j = 0; j < 8; j++) {
                server.print(oneWire.Address[i][j], HEX);
                server.print((j != 7) ? ":" : "");
              }
              server.printP(tdE);
              server.printP(trE);
            }
          }
          server.printP(tableEnd);
        }
        else { //admin oneWire
          P(oneWireSetupTableHeader) = "<table  class='mot'>\n"
                                       "<tr>\n"
                                       "<th width='30px'>ID</th>"
                                       "<th width='70px'>Description</th>"
                                       "<th width='70px'>Address</th>\n"
                                       "<th width='70px'>Offset</th>"
                                       "<th width='130px'>Log interval</th>"
                                       "<th width='100px'>Release ?</th>\n"
                                       "<th width='70px'>Save</th>"
                                       "</tr>\n";

          server.printP(oneWireSetupTableHeader);
          P(rbs) = "<td class='td'> <button onClick='releaseB(";
          P(rbe) = ")'>Realese</button></td>";
          P(saveButtonEnd) = ",'OW')>Save</button>";
          for (i = 0; i < sys.maxAlowedOneWire; i++) {
            if (!oneWire.isFree[i]) {
              server.printP(trS);
              server.printP(tdS);  server.print(i);                        server.printP(tdE);
              server.printP(tdS);
              for (j = 0; j < 8; j++) {
                server.print(oneWire.Address[i][j], HEX);
                server.print((j != 7) ? ":" : "");
              }
              server.printP(tdE);
              server.printP(tdS);  server.printP(tbS);   P(owDes) = "name='descriptionOW' value='"; server.printP(owDes);  server.print(oneWire.Description[i]); server.printP(e0); server.printP(tdE);
              server.printP(tdS);  server.printP(tbnRS); P(owOff) = "name='offsetOW' value='";      server.printP(owOff);  server.print(oneWire.Offset[i]); server.printP(e0); server.printP(tdE);
              server.printP(tdS);  server.printP(tbnLS); P(owLog) = "name='logIntervalOW' value='"; server.printP(owLog);  server.print(oneWire.logInterval[i]); server.printP(e0); server.printP(tdE);
              server.printP(rbs);  server.print(i);      server.printP(rbe);
              server.printP(tdS);  server.printP(saveButtonStart); server.print(i); server.printP(saveButtonEnd); server.printP(tdE);
            }
            server.printP(trE);
          }
          server.printP(tableEnd);
        }
      }
      else if (strcmp(valueFunc, "updateVal") == 0) { //update oneWire value
        P(ath) = "<table class='mot'>"
                 "<tr>"
                 "<th width='70px'>Temperature</th>"
                 "</tr>";
        server.printP(ath); //oneWire Table Header
        for (i = 0; i < sys.maxAlowedOneWire; i++) {
          if (!oneWire.isFree[i]) {
            server.printP(trS);
            server.printP(valTD);          server.print(oneWire.ValueTemp[i]);          server.printP(tdE);
            server.printP(trE);
          }
        }
        server.printP(tableEnd);
      }
      else if (strcmp(valueFunc, "releaseB") == 0) {
        repeat = server.readPOSTparam(name, 16, valueNum, 16);
        oneWireRelease(atoi(valueNum)); //save to EEPROM in function
        eeSave[ONEWIRE] = 1;
      }
      else if (strcmp(valueFunc, "scanAndLock") == 0) {
        scanOneWire(); //save to EEPROM in function
        eeSave[ONEWIRE] = 1;
      }
      else if (strcmp(valueFunc, "saveTableOW") == 0) {
        uint8_t i = 0;
        do {
          repeat = server.readPOSTparam(valueFunc, 16, valueNum, 16); //get element:id
          if (strcmp(valueFunc, "index") == 0) {
            i = atoi(valueNum);
          }
          else if (strcmp(valueFunc, "description") == 0) {
            if (strlen(valueNum) < MAX_ONE_WIRE_DESCRIPTION) sprintf(oneWire.Description[i], valueNum, i );
          }
          else if (strcmp(valueFunc, "offset") == 0)      {
            oneWire.Offset[i] = atof(valueNum);
          }
          else if (strcmp(valueFunc, "logInterval") == 0) {
            if (atoi(valueNum) >= 0)  oneWire.logInterval[i] = atoi(valueNum);
          }
        } while (repeat);

        eeSave[ONEWIRE] = 1;
      }

    }
  }
}

//Analog
void analogP(WebServer & server, WebServer::ConnectionType type, char *, bool) {
  //Put this on the top of every page to prevent hack
  if (!getPrivileges(server))   return;

  server.httpSuccess();
  if (type == WebServer::GET) { //http reqested to load a page
    server.printP(DOCTYPE);
    server.printP(defaultTitle);
    server.printP(headS);
    server.printP(style);
    server.printP(mmmStyle);
    server.printP(msmaStyle);
    server.printP(monScript);
    server.printP(headE);
    server.printP(bodyCen);
    if (user == USER) server.printP(UmainMenu); else server.printP(AmainMenu);
    server.print(htmlClock());
    server.printP(mainMenuClose);
    server.printP(monitorSubMenu);
    server.print("<div class='disp'>");
    server.printP(tableHolder);
    server.printP(valTableHolder);
    server.print("</div>");
    server.printP(endDoc);
  }

  if (type == WebServer::POST)//client send data to server
  {
    bool repeat;
    char name[16], valueFunc[16], valueNum[16];
    repeat = server.readPOSTparam(name, 16, valueFunc, 16);
    if (strcmp(name, "func") == 0) {

      if (strcmp(valueFunc, "updateClock") == 0) { //updating the clock
        server.print(getClockString()); //sent clock string
      }

      else if (strcmp(valueFunc, "updateTable") == 0) { //updating the main table
        if (user == USER) {
          P(athU) = "<table class='mot'>"
                    "<tr>"
                    "<th width='30px'>ID</th>"
                    "<th width='70px'>Description</th>"
                    "</tr>";

          server.printP(athU);

          for (i = 0; i < sys.maxAlowedAnalog; i++) {
            server.printP(trS);
            server.printP(tdS);          server.print(i);                        server.printP(tdE);
            server.printP(tdS);          server.print(analog.Description[i]);    server.printP(tdE);
            server.printP(trE);
          }
          server.printP(tableEnd);
        }
        else
        {
          P(athA) = "<table class='mot'>"
                    "<tr>"
                    "<th width='30px'>ID</th>"
                    "<th width='70px'>Description</th>"
                    "<th width='30px'>Vin</th>"
                    "<th width='70px'>Min</th>"
                    "<th width='70px'>Max</th>"
                    "<th width='70px'>Offset</th>"
                    "<th width='auto'>Log interval</th>"
                    "<th width='70px'>Save</th>"
                    "</tr>";
          server.printP(athA); //different header for Admin and for User
          P(saveButtonEnd) = ",'AN')>Save</button>";
          for (i = 0; i < sys.maxAlowedAnalog; i++) {
            server.printP(trS);
            server.printP(tdS);  server.print(i);                        server.printP(tdE);
            server.printP(tdS);  server.printP(tbS);   P(anDes) = "name='descriptionAN' value='"; server.printP(anDes);    server.print(analog.Description[i]); server.printP(e0); server.printP(tdE);
            //            server.printP(tdS);  server.printP(tbnLS);  P(anMul) = "name='mulAN' value='"; server.printP(anMul);           server.print(analog.Mul[i]);         server.printP(e0); server.printP(tdE);
            //            server.printP(tdS);  server.printP(tbnLS);  P(anDiv) = "name='divAN' value='"; server.printP(anDiv);           server.print(analog.Div[i]);         server.printP(e0); server.printP(tdE);
            server.printP(tdS);
            P(selV) = "<select name='Vmin' class='sel'>"; server.printP(selV);
            // P(oV1) = "<option value='1'";
            server.printP(ov1); if (analog.minVoltage[i] == 1) server.printP(selected); server.print(">1"); server.printP(optionE);
            server.printP(ov2); if (analog.minVoltage[i] == 0) server.printP(selected); server.print(">0"); server.printP(optionE);
            server.printP(selectE);

            server.printP(tdS);  server.printP(tbnLS);  P(anMin) = "name='minAN' value='"; server.printP(anMin);           server.print(analog.Min[i]);         server.printP(e0); server.printP(tdE);
            server.printP(tdS);  server.printP(tbnLS);  P(anMax) = "name='maxAN' value='"; server.printP(anMax);           server.print(analog.Max[i]);         server.printP(e0); server.printP(tdE);
            server.printP(tdS);  server.printP(tbnRS);  P(anOff) = "name='offsetAN' value='"; server.printP(anOff);        server.print(analog.Offset[i]);      server.printP(e0); server.printP(tdE);
            server.printP(tdS);  server.printP(tbnLS); P(anLog) = "name='logIntervalAN' value='"; server.printP(anLog);    server.print(analog.logInterval[i]); server.printP(e0); server.printP(tdE);
            //           server.printP(tdS);  server.print("<input type='text' class='val' readonly name='valueAN' value='"); server.print(analog.Value[i]); server.print("'/>"); server.printP(tdE);
            server.printP(tdS);  server.printP(saveButtonStart); server.print(i); server.printP(saveButtonEnd); server.printP(tdE);
            server.printP(trE);
          }
          server.printP(tableEnd);
        }
      }

      else if (strcmp(valueFunc, "updateVal") == 0) { //update analog value
        P(ath) = "<table class='mot'>"
                 "<tr>"
                 "<th width='70px'>Value</th>"
                 "</tr>";
        server.printP(ath); //analog Table Header
        for (i = 0; i < sys.maxAlowedAnalog; i++) {
          server.printP(trS);
          server.printP(valTD);          server.print(analog.Value[i]);          server.printP(tdE);
          server.printP(trE);
        }
        server.printP(tableEnd);
      }

      else if (strcmp(valueFunc, "saveTableAN") == 0) {
        uint8_t i = 0;
        do {
          repeat = server.readPOSTparam(valueFunc, 16, valueNum, 16); //get element:id
          if (strcmp(valueFunc, "index") == 0)
            i = atoi(valueNum);
          else if (strcmp(valueFunc, "description") == 0)  {
            if (strlen(valueNum) < MAX_ANALOG_DESCRIPTION) sprintf(analog.Description[i], valueNum, i );
          }
          else if (strcmp(valueFunc, "Vmin") == 0)          {
            if (atoi(valueNum) >= 0)  analog.minVoltage[i] = atoi(valueNum);
          }
          else if (strcmp(valueFunc, "min") == 0)          {
            if (atoi(valueNum) >= 0)  analog.Min[i] = atof(valueNum);
          }
          else if (strcmp(valueFunc, "max") == 0)          {
            if (atoi(valueNum) > 0 && atoi(valueNum) <= 3000 )  analog.Max[i] = atof(valueNum);
          }
          else if (strcmp(valueFunc, "mul") == 0)          {
            //         if (atoi(valueNum) > 0)  analog.Mul[i] = atof(valueNum);
          }
          else if (strcmp(valueFunc, "div") == 0)          {
            //        if (atoi(valueNum) > 0)  analog.Div[i] = atof(valueNum);
          }
          else if (strcmp(valueFunc, "offset") == 0)       {
            analog.Offset[i] = atof(valueNum);
          }
          else if (strcmp(valueFunc, "logInterval") == 0)  {
            if (atoi(valueNum) >= 0) analog.logInterval[i] = atoi(valueNum);
          }
        } while (repeat);
        //        Serial.println("Save analog:Done");
        calcM(i);
        calcB(i);
        eeSave[ANALOG] = 1;
      }

    }
  }
}
/*
//Humidity
void humidP(WebServer & server, WebServer::ConnectionType type, char *, bool) {
  //Put this on the top of every page to prevent hack
  if (!getPrivileges(server))   return;

  server.httpSuccess();
  if (type == WebServer::GET) { //http reqested to load a page
    server.printP(DOCTYPE);
    server.printP(defaultTitle);
    server.printP(headS);
    server.printP(style);
    server.printP(mmmStyle);
    server.printP(msmhStyle);
    server.printP(monScript);
    server.printP(headE);
    server.printP(bodyCen);
    if (user == USER) server.printP(UmainMenu); else server.printP(AmainMenu);
    server.print(htmlClock());
    server.printP(mainMenuClose);
    server.printP(monitorSubMenu);
    server.print("<div class='disp'>");
    server.printP(tableHolder);
    server.printP(valTableHolder);
    server.print("</div>");
    server.printP(endDoc);
  }

  if (type == WebServer::POST)//client send data to server
  {
    bool repeat;
    char name[16], valueFunc[16], valueNum[16];
    repeat = server.readPOSTparam(name, 16, valueFunc, 16);
    if (strcmp(name, "func") == 0) {

      if (strcmp(valueFunc, "updateClock") == 0) { //updating the clock
        server.print(getClockString()); //sent clock string
      }

      else if (strcmp(valueFunc, "updateTable") == 0) { //updating the main table
        if (user == USER) {
          P(athU) = "<table class='mot'>"
                    "<tr>"
                    "<th width='30px'>ID</th>"
                    "<th width='70px'>Description</th>"
                    "</tr>";

          server.printP(athU);

          for (i = 0; i < sys.maxAlowedHumid; i++) {
            server.printP(trS);
            server.printP(tdS);          server.print(i);                        server.printP(tdE);
            server.printP(tdS);          server.print(humid.Description[i]);    server.printP(tdE);
            server.printP(trE);
          }
          server.printP(tableEnd);
        }
        else
        {
          P(athA) = "<table class='mot'>"
                    "<tr>"
                    "<th width='30px'>ID</th>"
                    "<th width='70px'>Description</th>"
     //               "<th width='70px'>Mul</th>"
    //                "<th width='70px'>Div</th>"
                    "<th width='70px'>Offset</th>"
                    "<th width='auto'>Log interval</th>"
                    "<th width='70px'>Save</th>"
                    "</tr>";
          server.printP(athA); //different header for Admin and for User
          P(saveButtonEnd) = ",'HU')>Save</button>";
          for (i = 0; i < sys.maxAlowedHumid; i++) {
            server.printP(trS);
            server.printP(tdS);  server.print(i);                        server.printP(tdE);
            server.printP(tdS);  server.printP(tbS);   P(huDes) = "name='descriptionHU' value='"; server.printP(huDes);   server.print(humid.Description[i]); server.printP(e0); server.printP(tdE);
      //      server.printP(tdS);  server.printP(tbnLS);  P(HMul) = "name='mulAN' value='"; server.printP(anMul);           server.print(analog.Mul[i]);         server.printP(e0); server.printP(tdE);
     //       server.printP(tdS);  server.printP(tbnLS);  P(anDiv) = "name='divAN' value='"; server.printP(anDiv);           server.print(analog.Div[i]);         server.printP(e0); server.printP(tdE);
            server.printP(tdS);  server.printP(tbnRS);  P(huOff) = "name='offsetHU' value='"; server.printP(huOff);        server.print(humid.Offset[i]);      server.printP(e0); server.printP(tdE);
            server.printP(tdS);  server.printP(tbnLS); P(huLog) = "name='logIntervalHU' value='"; server.printP(huLog);  server.print(humid.logInterval[i]); server.printP(e0); server.printP(tdE);
            //           server.printP(tdS);  server.print("<input type='text' class='val' readonly name='valueAN' value='"); server.print(analog.Value[i]); server.print("'/>"); server.printP(tdE);
            server.printP(tdS);  server.printP(saveButtonStart); server.print(i); server.printP(saveButtonEnd); server.printP(tdE);
            server.printP(trE);
          }
          server.printP(tableEnd);
        }
      }

      else if (strcmp(valueFunc, "updateVal") == 0) { //update analog value
        P(ath) = "<table class='mot'>"
                 "<tr>"
                 "<th width='70px'>Value</th>"
                 "</tr>";
        server.printP(ath); //analog Table Header
        for (i = 0; i < sys.maxAlowedHumid; i++) {
          server.printP(trS);
          server.printP(valTD);          server.print(humid.Humid[i]);          server.printP(tdE);
          server.printP(trE);
        }
        server.printP(tableEnd);
      }

      else if (strcmp(valueFunc, "saveTableHU") == 0) {
        uint8_t i = 0;
        do {
          repeat = server.readPOSTparam(valueFunc, 16, valueNum, 16); //get element:id
          if (strcmp(valueFunc, "index") == 0)
            i = atoi(valueNum);
          else if (strcmp(valueFunc, "description") == 0)  {
            if (strlen(valueNum) < MAX_HUMID_DESCRIPTION) sprintf(humid.Description[i], valueNum, i );
          }
         else if (strcmp(valueFunc, "offset") == 0)       {
            humid.Offset[i] = atof(valueNum);
          }
          else if (strcmp(valueFunc, "logInterval") == 0)  {
            if (atoi(valueNum) >= 0) humid.logInterval[i] = atoi(valueNum);
          }
        } while (repeat);
        eeSave[HUMID] = 1;
      }

    }
  }
}
*/

//Digital
void digitalP(WebServer & server, WebServer::ConnectionType type, char *, bool) {
  //Put this on the top of every page to prevent hack
  if (!getPrivileges(server))   return;

  server.httpSuccess();
  if (type == WebServer::GET) { //http reqested to load a page
    server.printP(DOCTYPE);
    server.printP(defaultTitle);
    server.printP(headS);
    server.printP(style);
    server.printP(mmmStyle);
    server.printP(msmdStyle);
    server.printP(monScript);
    server.printP(headE);
    server.printP(bodyCen);
    if (user == USER) server.printP(UmainMenu); else server.printP(AmainMenu);
    server.print(htmlClock());
    server.printP(mainMenuClose);
    server.printP(monitorSubMenu);
    server.print("<div class='disp'>");
    server.printP(tableHolder);
    server.printP(valTableHolder);
    server.print("</div>");
    server.printP(endDoc);
  }

  if (type == WebServer::POST)//client send data to server
  {
    bool repeat;
    char name[16], valueFunc[16], valueNum[16];
    repeat = server.readPOSTparam(name, 16, valueFunc, 16);
    if (strcmp(name, "func") == 0) {

      if (strcmp(valueFunc, "updateClock") == 0) { //updating the oneWireTable
        server.print(getClockString()); //sent clock string
      }

      //Digital Table refresh
      else if (strcmp(valueFunc, "updateTable") == 0) { //updating the oneWireTable
        if (user == USER) {
          P(dgthA) = "<table class='mot'>\n"
                     "<tr>\n"
                     "<th width='30px'>ID</th>\n"
                     "<th width='70px'>Description</th>\n"
                     "</tr>";
          server.printP(dgthA);
          for (i = 0; i < sys.maxAlowedDigital; i++) {
            server.printP(trS);
            server.printP(tdS);        server.print(i);                                   server.printP(tdE);
            server.printP(tdS);        server.print(digital.Description[i]);              server.printP(tdE);
            server.printP(trE);
          }
          server.printP(tableEnd);
        }

        else {
          P(dgthA) = "<table  class='mot'>\n"
                     "<tr>\n"
                     "<th width='30px'>ID</th>\n"
                     "<th width='70px'>Description</th>\n"
                     "<th width='70px'>Counter</th>\n"
                     "<th width='130px'>Log interval</th>\n"
                     "<th width='70px'>Save</th>\n"
                     "</tr>\n";

          server.printP(dgthA);
          P(saveButtonEnd) = ",'DG')>Save</button>";
          for (i = 0; i < sys.maxAlowedDigital; i++) {

            server.printP(trS);
            server.printP(tdS);  server.print(i);                    server.printP(tdE);
            server.printP(tdS);  server.printP(tbS);   P(dgDes) = "name='descriptionDG' value='"; server.printP(dgDes);  server.print(digital.Description[i]); server.printP(e0); server.printP(tdE);
            server.printP(tdS);  server.printP(tbnLS);  P(dgCnt) = "name='counterDG' value='"; server.printP(dgCnt);      server.print(digital.Counter[i]);     server.printP(e0); server.printP(tdE);
            server.printP(tdS);  server.printP(tbnLS); P(dgLog) = "name='logIntervalDG' value='"; server.printP(dgLog);  server.print(digital.logInterval[i]); server.printP(e0); server.printP(tdE);
            server.printP(tdS);  server.printP(saveButtonStart); server.print(i); server.printP(saveButtonEnd); server.printP(tdE);
            server.printP(trE);
          }
          server.printP(tableEnd);
        }
      }

      else if (strcmp(valueFunc, "updateVal") == 0) { //
        P(ath) = "<table class='mot'>"
                 "<th width='70px'>State</th>"
                 "<th width='70px'>Counter</th>"
                 "</tr>";
        server.printP(ath); //analog Table Header
        for (i = 0; i < sys.maxAlowedDigital; i++) {
          server.printP(trS);
          server.printP(tdS); server.printP(digital.State[i] ? selHigh : selLow);  server.printP(tdE);
          server.printP(tdS); server.print(digital.Counter[i]);                   server.printP(tdE);
          server.printP(trE);
        }
        server.printP(tableEnd);
      }

      else if (strcmp(valueFunc, "saveTableDG") == 0) {
        uint8_t i = 0;
        do {
          repeat = server.readPOSTparam(valueFunc, 16, valueNum, 16); //get element:id
          if (strcmp(valueFunc, "index") == 0)             i = atoi(valueNum);
          else if (strcmp(valueFunc, "description") == 0) {
            if (strlen(valueNum) < MAX_DIGITAL_DESCRIPTION) sprintf(digital.Description[i], valueNum, i );
          }
          else if (strcmp(valueFunc, "counter") == 0)     {
            if (atoi(valueNum) >= 0)  digital.Counter[i] = atoi(valueNum);
          }
          else if (strcmp(valueFunc, "logInterval") == 0) {
            if (atoi(valueNum) >= 0) digital.logInterval[i] = atoi(valueNum);
          }
        } while (repeat);
        // EEPROM_writeAnything(digitalStart, digital);
        eeSave[DIGITAL] = 1;
      }
    }
  }
}

//Relay
void relayP(WebServer & server, WebServer::ConnectionType type, char *, bool) {
  //Put this on the top of every page to prevent hack
  if (!getPrivileges(server))   return;

  server.httpSuccess();
  if (type == WebServer::GET) { //http reqested to load a page
    server.printP(DOCTYPE);
    server.printP(defaultTitle);
    server.printP(headS);
    server.printP(style);
    server.printP(mmmStyle);
    server.printP(msmrStyle);
    server.printP(monScript);
    server.printP(headE);
    server.printP(bodyCen);
    if (user == USER) server.printP(UmainMenu); else server.printP(AmainMenu);
    server.print(htmlClock());
    server.printP(mainMenuClose);
    server.printP(monitorSubMenu);
    server.print("<div class='dispRL'>");
    server.printP(tableHolder);
    server.printP(valTableHolder);
    server.print("</div>");
    server.printP(endDoc);
  }

  if (type == WebServer::POST)//client send data to server
  {
    bool repeat;
    char name[16], valueFunc[16], valueNum[16];
    repeat = server.readPOSTparam(name, 16, valueFunc, 16);
    if (strcmp(name, "func") == 0) {

      if (strcmp(valueFunc, "updateClock") == 0) { //updating the oneWireTable
        server.print(getClockString()); //sent clock string
      }


      if (strcmp(valueFunc, "updateTable") == 0) { //updating the oneWireTable
        if (user == USER) {
          P(rth) = "<table class='mot'>"
                   "<tr>"
                   "<th width='30px'>ID</th>"
                   "<th width='70px'>Description</th>"
                   "</tr>";
          server.printP(rth);
          for (i = 0; i < sys.maxAlowedRelay; i++) {
            server.printP(trS);
            server.printP(tdS);     server.print(i);                                server.printP(tdE);
            server.printP(tdS);     server.print(relay.Description[i]);             server.printP(tdE);
            server.printP(trE);
          }
          server.printP(tableEnd);
        }
        else { //admin relay
          P(relaySetupTableHeader) = "<table  class='mot'>\n"
                                     "<tr>\n"
                                     "<th width='30px'>ID</th>\n"
                                     "<th width='70px'>Description</th>\n"
                                     "<th width='70px'>Normaly</th>\n"
                                     "<th width='70px'>Save</th>\n"
                                     "</tr>\n";
          server.printP(relaySetupTableHeader);
          P(saveButtonEnd) = ",'RL')>Save</button>";

          for (i = 0; i < sys.maxAlowedRelay; i++) {
            server.printP(trS);
            server.printP(tdS);  server.print(i);                    server.printP(tdE);
            server.printP(tdS);  server.printP(tbS);   P(rlDes) = "name='descriptionRL' value='"; server.printP(rlDes);  server.print(relay.Description[i]); server.printP(e0); server.printP(tdE);
            server.printP(tdS);
            P(relatState) = "<select name='normalyRL' class='sel'>"; server.printP(relatState);
            server.printP(ov1); if (relay.Normaly[i] == ON)  server.printP(selected); server.print(">On");  server.printP(optionE);
            server.printP(ov2); if (relay.Normaly[i] == OFF) server.printP(selected); server.print(">Off"); server.printP(optionE);
            server.printP(selectE);
            server.printP(tdE);
            server.printP(tdS);  server.printP(saveButtonStart); server.print(i); server.printP(saveButtonEnd); server.printP(tdE);
            server.printP(trE);
          }
          server.printP(tableEnd);
        }
      }

      else if (strcmp(valueFunc, "updateVal") == 0) { //
        P(ath) = "<table class='mot'>"
                 "<tr>"
                 "<th width='70px'>On/Off</th>"
                 "<th width='70px'>State</th>"
                 "</tr>";
        server.printP(ath); //analog Table Header
        P(btn1S) = "<button type='button' onclick=toggleRelay(";
        P(btn1E) = ")>Toggle</button>";
        for (i = 0; i < sys.maxAlowedRelay; i++) {
          server.printP(trS);
          server.printP(tdS);     server.printP(btn1S); server.print(i);  server.printP(btn1E);  server.printP(tdE);
          server.printP(tdS);     server.printP(relay.State[i] ? selHigh : selLow );              server.printP(tdE);
          server.printP(trE);
        }
        server.printP(tableEnd);
      }

      else if (strcmp(valueFunc, "toggleRelay") == 0) { //
        repeat = server.readPOSTparam(valueFunc, 16, valueNum, 16);
        toggleRelay(atoi(valueNum));
      }
      else if (strcmp(valueFunc, "saveTableRL") == 0) {
        uint8_t i = 0;
        do {
          repeat = server.readPOSTparam(valueFunc, 16, valueNum, 16); //get element:id
          if (strcmp(valueFunc, "index") == 0)              i = atoi(valueNum);
          else if (strcmp(valueFunc, "description") == 0) {
            if (strlen(valueNum) < MAX_DIGITAL_DESCRIPTION) sprintf(digital.Description[i], valueNum, i );
          }
          else if (strcmp(valueFunc, "normaly") == 0)     {
            relay.Normaly[i] = atoi(valueNum);
            setRelayToNormaly(i);
          }
        } while (repeat);
        // EEPROM_writeAnything(digitalStart, digital);
        eeSave[RELAY] = 1;
      }
    }
  }
}

//Rules
void rulesP(WebServer & server, WebServer::ConnectionType type, char *, bool) {
  //Put this on the top of every page to prevent hack
  if (!getPrivileges(server))   return; //not recognized user - this line can be deleted

  if ( (user == ADMIN) || (user == FACTORY) ) { //Unauthorized

    server.httpSuccess();
    if (type == WebServer::GET) { //http reqested to load a page
      server.printP(DOCTYPE);
      server.printP(defaultTitle);
      server.printP(headS);
      server.printP(style);
      server.printP(mmsStyle); //Main Menu -> Setup
      server.printP(ssmruStyle); //                    Setup -> Sub Menu -> RUles
      server.printP(rulesScript);
      server.printP(headE);
      server.printP(bodyCen);
      server.printP(AmainMenu);
      server.print(htmlClock());
      server.printP(mainMenuClose);
      server.printP(setupSubMenu);
      /*
            P(m1) = "<center><b>Rule:</b><select id='ruleID' class='sel' onchange=loadRule(this.value)></center>"; server.printP(m1);
            for (j = 0; j < sys.maxAlowedRules; j++) {
              P(ruleIDsel) = "<option value='";
              server.printP(ruleIDsel);
              server.print(j);
              server.printP(e0);
              server.print(j);
              server.printP(optionE);
            }  server.printP(selectE);
      */
      //    server.print("<div class='disp'>");
      server.printP(tableHolder);
      //   server.print("</div>");
      server.printP(endDoc);
    }

    if (type == WebServer::POST)//client send data to server
    {
      bool repeat;
      char name[16], valueFunc[16], valueNum[17];
      repeat = server.readPOSTparam(name, 16, valueFunc, 16);
      if (strcmp(name, "func") == 0) {

        if (strcmp(valueFunc, "updateClock") == 0) { //updating the oneWireTable
          server.print(getClockString()); //sent clock string
        }
        else if (strcmp(valueFunc, "loadValue") == 0) { //updating the oneWireTable
          server.readPOSTparam(name, 16, valueFunc, 16);
          server.readPOSTparam(name, 16, valueNum, 16);
          if (atoi(valueNum) != NA) {
            if (strcmp(valueFunc, "OW") == 0) server.print(oneWire.ValueTemp[atoi(valueNum)]);
            else if (strcmp(valueFunc, "AN") == 0) server.print(analog.Value[atoi(valueNum)]);
            else if (strcmp(valueFunc, "DG") == 0) server.print(digital.Counter[atoi(valueNum)]);
          }
          else server.print("N/A");
        }

        else if (strcmp(valueFunc, "updateTable") == 0) {
          server.readPOSTparam(name, 16, valueNum, 16);
          i = atoi(valueNum);
          TimeElements tmp;
          P(m1) = "<center><b>Rule:</b><select id='ruleID' class='sel' onchange=loadRule(this.value)>"; server.printP(m1); /*onchange*/
          for (j = 0; j < sys.maxAlowedRules; j++) {
            P(ruleIDsel) = "<option value='";
            server.printP(ruleIDsel);
            server.print(j); if (i == j) {
              server.print("'");
              server.printP(selected);
              server.print(">");
            }
            else {
              server.printP(e0);
            }
            server.print(j);
            server.printP(optionE);
          }  server.printP(selectE);
          server.print(" ");
          server.printP(tbS);   P(ruleDes) = "name='description' value='"; server.printP(ruleDes);  server.print(rule.description[i]); server.printP(e0);
          server.print("</center>");
          P(opS) = "<option ";
          P(value) = "value='";

          P(ruleHeader) = "<b><table  class='mot' style='float:none;text-align:center;'>\n"
                          "<tr>\n"
                          "<td class='td' >ID</td>" //ID
                          "<td class='td' >State</td>" //State
                          "<td class='td' ></td>"
                          "<td class='td' colspan='7'>Schedule</td>"
                          "<td class='td' >Type</td>"
                          "<td class='td' >ID</td>"
                          "<td class='td' >Min</td>"
                          "<td class='td' >Max</td>"
                          "<td class='td' >Hyst</td>"
                          "<td class='td' >Value</td>"
                          //            "<td class='td' >Trig Condition</td>"
                          "</tr>\n";
          server.printP(ruleHeader);
          P(ovNA) = ("<option value='N/A'>N/A");
          P(ov) = ("</option>");
          P(dtS) = ("<input type='datetime-local' style='background-color:#ECECEC; border-color:#CF0000; border-width:1px; border-style:solid;' '");

          server.printP(trS);
          server.print("<td rowspan='4'>");  server.print(i);    server.printP(tdE);
          server.print("<td rowspan='4'>");  server.printP(cbS); server.print(rule.state[i] ? "checked" : "" ); server.printP(Name); P(state) = "state"; server.printP(state); server.printP(e0); server.printP(tdE);
          server.printP(tdSS); server.print("Start:"); server.printP(tdE);
          server.print("<td colspan='7'>"); server.printP(dtS); server.printP(Name); P(start) = "start' value='"; server.printP(start);

          if (rule.startTime[i] != 0) {
            breakTime(rule.startTime[i], tmp);
            server.print(tmp.Year + 1970); server.print("-");
            if (tmp.Month < 10)  server.print("0"); server.print(tmp.Month); server.print("-");
            if (tmp.Day < 10)    server.print("0"); server.print(tmp.Day); server.print("T");
            if (tmp.Hour < 10)   server.print("0"); server.print(tmp.Hour); server.print(":");
            if (tmp.Minute < 10) server.print("0"); server.print(tmp.Minute); server.print("'>");
          }
          else server.print("'>");
          server.printP(tdE);

          server.printP(tdSS); server.print("Analog:"); server.printP(tdE);

          //analog
          digitalWrite(18, HIGH);
          server.printP(tdS);

          P(selN) = "<select name='anID' class='sel' onchange=loadValue('AN',this.value)>"; server.printP(selN); server.printP(na); if (rule.analog[i] == NA) server.printP(selected); server.printP(NAe); server.printP(optionE);
          for (j = 0; j < sys.maxAlowedAnalog; j++) {
            server.printP(opS); if (j == rule.analog[i]) server.printP(selected);
            server.printP(value);
            server.print(j);
            server.printP(e0);
            server.print(j);
            server.printP(optionE);
          }  server.printP(selectE);
          server.printP(tdE);

          server.printP(tdS);   server.printP(tbnRS); server.printP(Name); P(anMin) = "anMin"; server.printP(anMin); server.printP(tbM); server.print(rule.analogMin[i]); server.printP(e0); server.printP(tdE);
          server.printP(tdS);   server.printP(tbnRS); server.printP(Name); P(anMax) = "anMax"; server.printP(anMax); server.printP(tbM); server.print(rule.analogMax[i]); server.printP(e0); server.printP(tdE);
          server.printP(tdS);   server.printP(tbnRS); server.printP(Name); P(anHist) = "anHist"; server.printP(anHist); server.printP(tbM); server.print(rule.analogHister[i]); server.printP(e0); server.printP(tdE);
          server.printP(tdS);   server.print("<p id='ANValue' class='p' >"); if (rule.analog[i] == NA) server.print("N/A"); else server.print(analog.Value[rule.analog[i]]); server.print("</p>");  server.printP(tdE);
          server.printP(trE);

          server.printP(trS);
          server.printP(tdSS); server.print("End:"); server.printP(tdE);
          server.print("<td colspan='7'>"); server.printP(dtS); server.printP(Name); P(end) = "end' value='"; server.printP(end);

          if (rule.endTime[i] != 0) {
            breakTime(rule.endTime[i], tmp);
            server.print(tmp.Year + 1970); server.print("-");
            if (tmp.Month < 10)  server.print("0"); server.print(tmp.Month); server.print("-");
            if (tmp.Day < 10)    server.print("0"); server.print(tmp.Day); server.print("T");
            if (tmp.Hour < 10)   server.print("0"); server.print(tmp.Hour); server.print(":");
            if (tmp.Minute < 10) server.print("0"); server.print(tmp.Minute); server.print("'>");
          }
          else server.print("'>");
          server.printP(tdE);

          //oneWire
          server.printP(tdSS); server.print("OneWire:"); server.printP(tdE);

          server.printP(tdS);
          P(m0) = "<select name='owID' class='sel' onchange=loadValue('OW',this.value)>"; server.printP(m0); server.printP(na); if (rule.oneWire[i] == NA) server.printP(selected); P(naV) = ">N/A"; server.printP(naV); server.printP(optionE);
          for (j = 0; j < sys.maxAlowedOneWire; j++) {
            if (!oneWire.isFree[j]) {
              server.printP(opS); if (j == rule.oneWire[i]) server.printP(selected);
              server.printP(value);
              server.print(j);
              server.printP(e0);
              server.print(j);
              server.printP(optionE);
            }
          }  server.printP(selectE);
          server.printP(tdE);

          //      server.printP(tdS);   server.printP(tbnRS); server.printP(Name); P(owID) = "owID"; server.printP(owID); server.printP(tbM); server.print(rule.analogMin[i]); server.printP(e0); server.printP(tdE);

          server.printP(tdS);   server.printP(tbnRS); server.printP(Name); P(owMin) = "owMin"; server.printP(owMin); server.printP(tbM); server.print(rule.oneWireMin[i]); server.printP(e0); server.printP(tdE);
          server.printP(tdS);   server.printP(tbnRS); server.printP(Name); P(owMax) = "owMax"; server.printP(owMax); server.printP(tbM); server.print(rule.oneWireMax[i]); server.printP(e0);  server.printP(tdE);
          server.printP(tdS);   server.printP(tbnRS); server.printP(Name); P(owHist) = "owHist"; server.printP(owHist); server.printP(tbM); server.print(rule.oneWireHister[i]); server.printP(e0); server.printP(tdE);
          server.printP(tdS);   server.print("<p id='OWValue' class='p'>"); if (rule.oneWire[i] == NA) server.print("N/A"); else server.print(oneWire.ValueTemp[rule.oneWire[i]]);  server.print("</p>"); server.printP(tdE);
          server.printP(trE);

          server.printP(trS);
          P(dailyHeader) = "<td></td>"
                           "<td>Su</td>"
                           "<td>Mo</td>"
                           "<td>Tu</td>"
                           "<td>We</td>"
                           "<td>Th</td>"
                           "<td>Fr</td>"
                           "<td>St</td>";
          server.printP(dailyHeader);
          //digital
          server.printP(tdSS); server.print("Digital:"); server.printP(tdE);

          server.printP(tdS);
          P(selNdg) = "<select name='dgID' class='sel' onchange=loadValue('DG',this.value)>"; server.printP(selNdg); server.printP(na); if (rule.digital[i] == NA) server.printP(selected); server.printP(NAe); server.printP(optionE);
          for (j = 0; j < sys.maxAlowedDigital; j++) {
            server.printP(opS); if (j == rule.digital[i]) server.printP(selected);
            server.printP(value);
            server.print(j);
            server.printP(e0);
            server.print(j);
            server.printP(optionE);
          }  server.printP(selectE);  server.printP(tdE);

          //          server.printP(tdS);   server.printP(tbnRS); server.printP(Name); P(dgID) = "dgID"; server.printP(dgID); server.printP(tbM); server.print(rule.analogMin[i]); server.printP(e0); server.printP(tdE);
          server.printP(tdS);   server.printP(tbnLS); P(dgCnt) = "name='dgCnt' value='"; server.printP(dgCnt); server.print(rule.digitalCount[i]); server.printP(e0); server.printP(tdE);
          server.printP(tdS);
          P(dgState) = "<select name='dgState' class='sel'>"; server.printP(dgState);
          server.printP(na); if (rule.digital[i] == NA); server.printP(selected); server.printP(NAe); server.printP(optionE);
          server.printP(ov1); if (rule.digitalState[i] == 1) server.printP(selected); server.print(">HIGH"); server.printP(optionE);
          server.printP(ov2); if (rule.digitalState[i] == 0) server.printP(selected); server.print(">LOW");  server.printP(optionE);
          server.printP(selectE);
          server.printP(tdS);  server.printP(tdE);
          server.printP(tdS);  server.print("<p id='DGValue' class='p'>"); if (rule.digital[i] == NA) server.print("N/A"); else server.print(digital.Counter[rule.digital[i]]);  server.print("</p>"); server.printP(tdE);
          server.printP(tdE);
          server.printP(trE);

          server.printP(trS);
          server.printP(tdSS); server.print("Repeat:"); server.printP(tdE);
          //daily repate'
          P(checked) = " checked";
          server.printP(tdS); server.printP(cbS); if (rule.weekDay[i][0]) server.printP(checked); server.printP(Name); P(su) = "su"; server.printP(su);  server.printP(e0); server.printP(tdE);
          server.printP(tdS); server.printP(cbS); if (rule.weekDay[i][1]) server.printP(checked); server.printP(Name); P(mu) = "mo"; server.printP(mu);  server.printP(e0); server.printP(tdE);
          server.printP(tdS); server.printP(cbS); if (rule.weekDay[i][2]) server.printP(checked); server.printP(Name); P(tu) = "tu"; server.printP(tu);  server.printP(e0); server.printP(tdE);
          server.printP(tdS); server.printP(cbS); if (rule.weekDay[i][3]) server.printP(checked); server.printP(Name); P(we) = "we"; server.printP(we);  server.printP(e0); server.printP(tdE);
          server.printP(tdS); server.printP(cbS); if (rule.weekDay[i][4]) server.printP(checked); server.printP(Name); P(th) = "th"; server.printP(th);  server.printP(e0); server.printP(tdE);
          server.printP(tdS); server.printP(cbS); if (rule.weekDay[i][5]) server.printP(checked); server.printP(Name); P(fr) = "fr"; server.printP(fr);  server.printP(e0); server.printP(tdE);
          server.printP(tdS); server.printP(cbS); if (rule.weekDay[i][6]) server.printP(checked); server.printP(Name); P(sa) = "sa"; server.printP(sa);  server.printP(e0); server.printP(tdE);

          server.printP(tdSS); server.print("Relay:"); server.printP(tdE);

          server.printP(tdS);
          P(relay) = "<select name='relay' class='sel'>"; server.printP(relay); server.printP(na); if (rule.relay[i] == NA) server.printP(selected); server.printP(NAe); server.printP(optionE);
          for (j = 0; j < sys.maxAlowedRelay; j++) {
            server.printP(opS); if (j == rule.relay[i]) server.printP(selected);
            server.printP(value);
            server.print(j);
            server.printP(e0);
            server.print(j);
            server.printP(optionE);
          }  server.printP(selectE);  server.printP(tdE);
          server.printP(tdSS); server.print("Action:");
          server.printP(tdE);


          //     server.printP(tdS);   server.printP(tbnRS); server.printP(Name); P(relay) = "relay"; server.printP(relay); server.printP(tbM); server.print(rule.analogMin[i]); server.printP(e0); server.printP(tdE);

          server.printP(tdS);
          P(relatState) = "<select name='relayState' class='sel'>"; server.printP(relatState);
          //     server.printP(na); if (rule.relayAction[i] == NA); server.printP(selected); server.printP(NAe); server.printP(optionE);
          server.printP(ov1); if (rule.relayAction[i] == ON) server.printP(selected); server.print(">On"); server.printP(optionE);
          server.printP(ov2); if (rule.relayAction[i] == OFF) server.printP(selected); server.print(">Off");  server.printP(optionE);
          server.printP(selectE);
          server.printP(tdE);

          //        server.printP(tdSS); server.print("Mail:"); server.printP(tdE);
          //        server.printP(tdS); server.print("x"); server.printP(tdE);
          //        server.printP(tdS); server.print(""); server.printP(tdE);
          server.printP(trE);
          server.printP(trS);
          server.print("<td class='td' colspan='11' >");   P(btn1S) = "<button type='button' onclick=postRow("; server.printP(btn1S); server.print(i); P(btn1E) = ")>Apply</button>"; server.printP(btn1E); server.printP(tdE);
          //       server.print("<td class='td' colspan='5' >");   P(remS) = "<button type='button' onclick=refreshVal("; server.printP(remS); server.print(i);  P(remE) = ")>Refresh Val</button>"; server.printP(remE);  server.printP(tdE);
          server.printP(trE);
          server.printP(tableEnd);
          //      server.print("<hr>");
          //       }
          //     } //end of isFree
          //      P(button) = "<button type='button' onclick=addRule()>Add Rule</button>"; server.printP(button);
          //      digitalWrite(18, LOW);
        }
        /*
                else if (strcmp(valueFunc, "addRule") == 0) {
                  if (rule.rulesCount < sys.maxAlowedRules)
                    addRule();
                  //EEPROM_writeAnything(ruleStart, rule);
                  eeSave[RULES] = 1;
                }
                else if (strcmp(valueFunc, "removeRule") == 0) {
                  repeat = server.readPOSTparam(valueFunc, 16, valueNum, 17); //get element:id
                  if (rule.rulesCount > 0) {
                    //     Serial.print("atoi(valueNum)"); Serial.println(atoi(valueNum));
                    removeRule(atoi(valueNum));
                  }
                  //    EEPROM_writeAnything(ruleStart, rule);
                  eeSave[RULES] = 1;
                }
        */
        else if (strcmp(valueFunc, "saveRule") == 0) {
          uint8_t i = 0;
          do {
            repeat = server.readPOSTparam(valueFunc, 16, valueNum, 17); //get element:id
            if (strcmp(valueFunc, "index") == 0)       {
              i = atoi(valueNum);
              //           ruleDeActivate(i); //deactivate the rule before we set a new settings to it
              //clearRule(i); //clear all settings before saving any new ones
            }
            else if (strcmp(valueFunc, "description") == 0)  {
              if (strlen(valueNum) < MAX_RULE_DESCRIPTION) sprintf(rule.description[i], valueNum, i );
            }

            else if (strcmp(valueFunc, "state") == 0)  {
              if (strcmp(valueNum, "true") == 0) rule.state[i] = true;
              else if (strcmp(valueNum, "false") == 0) {
                setRelayToNormaly(i); //rule is turned off - go back to normaly operationm relay is back to it's normal state
                rule.state[i] = false;
              }
            }
            else if (strcmp(valueFunc, "start") == 0)  {
              //       Serial.println(valueNum);
              clearTimeElements(&startElementsTmp);
              parseTime(valueNum, START);
            }
            else if (strcmp(valueFunc, "end") == 0)    {
              clearTimeElements(&endElementsTmp);
              parseTime(valueNum, END);
            }

            else if (strcmp(valueFunc, "sun") == 0)    {
              //         Serial.print("i: ");Serial.print(i); Serial.print(" Sunday:"); Serial.println(valueNum);
              rule.weekDay[i][0] = ((strcmp(valueNum, "true") == 0) ? true : false);
            }
            else if (strcmp(valueFunc, "mon") == 0)    {
              //         Serial.print("i: ");Serial.print(i); Serial.print(" Monday:"); Serial.println(valueNum);
              rule.weekDay[i][1] = ((strcmp(valueNum, "true") == 0) ? true : false);
            }
            else if (strcmp(valueFunc, "tus") == 0)    {
              //         Serial.print("i: ");Serial.print(i); Serial.print(" Tusday:"); Serial.println(valueNum);
              rule.weekDay[i][2] = ((strcmp(valueNum, "true") == 0) ? true : false);
            }
            else if (strcmp(valueFunc, "wed") == 0)    {
              //    Serial.print("i: ");Serial.print(i); Serial.print(" Wednesday:"); Serial.println(valueNum);
              rule.weekDay[i][3] = ((strcmp(valueNum, "true") == 0) ? true : false);
              //    Serial.println(rule.weekDay[i][3]);
            }
            else if (strcmp(valueFunc, "thu") == 0)    {
              //         Serial.print("i: ");Serial.print(i); Serial.print(" Thursday:"); Serial.println(valueNum);
              rule.weekDay[i][4] = ((strcmp(valueNum, "true") == 0) ? true : false);
            }
            else if (strcmp(valueFunc, "fri") == 0)    {
              //          Serial.print("i: ");Serial.print(i); Serial.print(" Friday:"); Serial.println(valueNum);
              rule.weekDay[i][5] = ((strcmp(valueNum, "true") == 0) ? true : false);
            }
            else if (strcmp(valueFunc, "sat") == 0)    {
              //          Serial.print("i: ");Serial.print(i); Serial.print(" Saturday:"); Serial.println(valueNum);
              rule.weekDay[i][6] = ((strcmp(valueNum, "true") == 0) ? true : false);
            }

            else if (strcmp(valueFunc, "owID") == 0)   {
              if (atoi(valueNum) == NA) rule.oneWire[i] = NA;
              else if (atoi(valueNum) >= 0 && (atoi(valueNum) < sys.maxAlowedOneWire))    rule.oneWire[i] = atoi(valueNum);
            }
            else if (strcmp(valueFunc, "owMin") == 0)  {
              /* if (atof(valueNum) >= 0)  */rule.oneWireMin[i] = atof(valueNum);
            }
            else if (strcmp(valueFunc, "owMax") == 0)  {
              if (atof(valueNum) >= 0)   rule.oneWireMax[i] = atof(valueNum);
            }
            else if (strcmp(valueFunc, "owHist") == 0) {
              if (atof(valueNum) >= 0)  rule.oneWireHister[i] = atof(valueNum);
            }
            else if (strcmp(valueFunc, "anID") == 0)   {
              if (atoi(valueNum) == NA) rule.analog[i] = NA;
              else if (atoi(valueNum) >= 0 && (atoi(valueNum) < sys.maxAlowedAnalog)) rule.analog[i] = atoi(valueNum);
            }
            else if (strcmp(valueFunc, "anMin") == 0)  {
              /*if (atof(valueNum) >= 0)*/ rule.analogMin[i] = atof(valueNum);
            }
            else if (strcmp(valueFunc, "anMax") == 0)  {
              if (atof(valueNum) >= 0)  rule.analogMax[i] = atof(valueNum);
            }
            else if (strcmp(valueFunc, "anHist") == 0) {
              if (atof(valueNum) >= 0)  rule.analogHister[i] = atof(valueNum);
            }
            else if (strcmp(valueFunc, "dgID") == 0)   {
              if (atoi(valueNum) == NA) rule.digital[i] = NA;
              else if (atoi(valueNum) >= 0 && (atoi(valueNum) < sys.maxAlowedDigital))    rule.digital[i] = atoi(valueNum);
            }
            else if (strcmp(valueFunc, "dgCnt") == 0)  {
              if (atoi(valueNum) >= 0)  rule.digitalCount[i] = atoi(valueNum);
            }
            else if (strcmp(valueFunc, "dgState") == 0) {
              if (atoi(valueNum) == NA) rule.digitalState[i] = NA;
              else rule.digitalState[i] = atoi(valueNum);
            }
            else if (strcmp(valueFunc, "relay") == 0)  {
              if (atoi(valueNum) == NA) {
                setRelayOff(rule.relay[i]);
                rule.relay[i] = NA;
              }
              else rule.relay[i] = atoi(valueNum);
            }
            else if (strcmp(valueFunc, "relayState") == 0)  {
              rule.relayAction[i] = atoi(valueNum);
              //             Serial.print("rule.relayAction[i] :"); Serial.println(rule.relayAction[i]);
            }
            else if (strcmp(valueFunc, "mail") == 0)   {
              /*mail report action*/
            }
          } while (repeat);
          setStartEnd(i);
          //     EEPROM_writeAnything(ruleStart, rule);
          eeSave[RULES] = 1;
        }
      }
    }
  }
  else {
    server.httpUnauthorized();
    return;
  }
}
void systemP(WebServer & server, WebServer::ConnectionType type, char *, bool) {
  if (!getPrivileges(server))   return;
  if ((user == ADMIN) || (user == FACTORY)) {
    server.httpSuccess();
    if (type == WebServer::GET) { //http reqested to load a page
      server.printP(DOCTYPE);
      server.printP(defaultTitle);
      server.printP(headS);
      server.printP(style);
      server.printP(mmsStyle); //Main Menu -> Setup
      server.printP(ssmsyStyle); //                 Setup -> Sub Senu -> system
      server.printP(systemSetupScript);
      server.printP(headE);
      server.printP(bodyCen);
      server.printP(AmainMenu);
      server.print(htmlClock());
      server.printP(mainMenuClose);
      server.printP(setupSubMenu);

      //   server.print("<center>");
      server.printP(timeDateTableHolder);
      server.printP(netTableHolder);
      server.printP(SNMPTableHolder);
      server.printP(PrivilegesTableHolder);
      if (user == FACTORY) {
        server.printP(factoryTableHolder);
      }
      // server.print("</center>");

      server.printP(endDoc);
    }

    if (type == WebServer::POST)//client send data to server
    {
      bool repeat;
      char name[16], valueFunc[17], valueNum[17];
      repeat = server.readPOSTparam(name, 17, valueFunc, 17);

      if (strcmp(valueFunc, "updateClock") == 0) { //updating the oneWireTable
        server.print(getClockString()); //sent clock string
      }

      else if (strcmp(valueFunc, "timeDate") == 0) {
        P(m0) = "<table id='timeDate' class='settingsTable'>"
                "<tr style='height:20px'>"
                "<td colspan='4' style='text-align:center;'><b>Time & Date Settings</b></td>";
        server.printP(m0);
        P(dtS) = ("<input type='datetime-local' style='background-color:#ECECEC; border-color:#CF0000; border-width:1px; border-style:solid;' '");
        server.printP(trS);
        P(m1) = "<td class='td' style='text-align:left' >Time & Date:</td>"; server.printP(m1);
        P(m2) = "<td class='td' style='text-align:right' colspan='3'>"; server.printP(m2);
        server.printP(dtS); server.printP(Name); P(start) = "clockField' value='"; server.printP(start);
        server.print(year()); server.print("-");
        if (month() < 10)  server.print("0"); server.print(month()); server.print("-");
        if (day() < 10)    server.print("0"); server.print(day()); server.print("T");
        if (hour() < 10)   server.print("0"); server.print(hour()); server.print(":");
        if (minute() < 10) server.print("0"); server.print(minute()); server.print("'>");
        server.printP(tdE);
        server.printP(trE);
        server.printP(trS);
        P(m3) = "<td class='td' style='text-align:left'>Time Server</td>"; server.printP(m3);
        P(m4) = "<td class='td' style='text-align:left'><input type='text' class='text-boxIP' maxlength='15' style='width:105px'"; server.printP(m4);  server.printP(Name); server.print("timeserver"); server.printP(tbM); for (j = 0; j < 4; j++) {
          server.print(netConf.timeServer[j]);
          server.print((j != 3) ? "." : "");
        } server.printP(e0); server.printP(tdE);
        P(m5) = "<td class='td' style='text-align:left' colspan='2'><button type='button' style='width:100px' onclick=updateTime()>Sync Time</button>"; server.printP(m5);  server.printP(tdE);
        server.printP(trE);
        server.printP(trS);
        P(m6) = "<td class='td' style='text-align:left'>NTP sync Interval</td>"; server.printP(m6);
        server.printP(tdSe); P(ntpTB) = "<input type='number' max='9999' min='0' step='1' class='num-box' name='ntpSyncInterval' style='width:105px;' value='"; server.printP(ntpTB); server.print(netConf.NTPSyncInterval); server.printP(e0); server.printP(tdE);

        server.printP(tdSe); P(TZ) = "Time Zone"; server.printP(TZ); server.printP(tdE);
        P(m7) = "<td class='td'><input type='text' class='text-box' style='width:20px' maxlength='2' name='timezone' value='"; server.printP(m7);  server.print(netConf.timeZone); server.printP(e0); server.printP(tdE);
        server.printP(trE);
        P(m8) = "<td class='td' style='text-align:right' colspan='4'><button type='button' onclick=saveTimeAndDate() class='save'>Save</button></td>"; server.printP(m8);
        server.printP(tableEnd);
      }

      else if (strcmp(valueFunc, "net") == 0) {
        P(t1) = "<table id='net' class='settingsTable'>"
                "<tr style='height:20px'>"
                "<td colspan='4' style='text-align:center;'><b>Network Settings</b></td>";
        server.printP(t1);

        server.printP(trS);
        server.printP(tdSe);  P(hn) = "Host Name"; server.printP(hn); server.printP(tdE);
        server.printP(tdSe);  server.print(netConf.hostName); server.printP(tdE);
        server.printP(trE);

        server.printP(trS);
        server.printP(tdSe);
        P(dhcpS) = "<select onchange=dhcpChange() style='background-color:#ECECEC; border-color=#CF0000;' name='dhcp'>"; server.printP(dhcpS);
        P(vOff) = "<option value='1'"; server.printP(vOff);  if (netConf.dhcp)  {
          server.printP(selected);
        } server.print(">"); server.print("DHCP"); server.printP(optionE);
        P(vOn) = "<option value='0'"; server.printP(vOn); if (!netConf.dhcp) {
          server.printP(selected);
        } server.print(">"); server.print("STATIC"); server.printP(optionE);
        server.printP(selectE);
        server.printP(tdE);

        server.print("<td class='tdSetup' id=netIP>");
        P(tbSIP1) = ("<input type='text' class='text-boxIP' maxlength='15' ");
        server.printP(tbSIP1); server.printP(Name); server.print("ip"); server.printP(tbM); for (j = 0; j < 4; j++) {
          server.print(netConf.ip[j]);
          server.print((j != 3) ? "." : "");
        }
        server.print("' ");
        if (netConf.dhcp) server.print("style='display:none' "); //dhcp is on - do not show the text box
        server.print(">");
        server.print("<d id='textIP'"); if (!netConf.dhcp) server.print("style='display:none'");  server.print(">"); for (j = 0; j < 4; j++) {
          server.print(netConf.ip[j]);
          server.print((j != 3) ? "." : "");
        }  server.print("</d>");
        server.printP(tdE);

        server.printP(tdSe); P(wp) = "Port"; server.printP(wp); server.printP(tdE);
        server.printP(tdSe); server.printP(tbnLS); P(wpTB) = "name='webPort' style='width:50px;' value='"; server.printP(wpTB); server.print(netConf.webPort); server.printP(e0); server.printP(tdE);
        server.printP(trE);

        server.printP(trS);
        server.printP(tdSe); server.print("Subnet Mask"); server.printP(tdE);
        server.printP(tdSe); server.printP(tbSIP); server.printP(Name); server.print("mask"); server.printP(tbM); for (j = 0; j < 4; j++) {
          server.print(netConf.mask[j]);
          server.print((j != 3) ? "." : "");
        } server.printP(e0); server.printP(tdE);
        server.printP(trE);

        server.printP(trS);
        server.printP(tdSe); server.print("Default Gateway"); server.printP(tdE);
        server.printP(tdSe); server.printP(tbSIP); server.printP(Name); server.print("gateway"); server.printP(tbM); for (j = 0; j < 4; j++) {
          server.print(netConf.gateway[j]);
          server.print((j != 3) ? "." : "");
        } server.printP(e0); server.printP(tdE);
        server.printP(trE);

        server.printP(tdSe); server.print("DNS"); server.printP(tdE);
        server.printP(tdSe); server.printP(tbSIP); server.printP(Name); server.print("dns"); server.printP(tbM); for (j = 0; j < 4; j++) {
          server.print(netConf.dns[j]);
          server.print((j != 3) ? "." : "");
        } server.printP(e0); server.printP(tdE);
        server.printP(trE);



        server.printP(trS);
        P(macAdd) = "<td class='tdSetup'>MAC Address</td>"; server.printP(macAdd);
        server.printP(tdSe);
        for (j = 0; j < 6; j++) {
          if (sys.mac[j] < 10) server.print("0");
          server.print(sys.mac[j], HEX);
          server.print((j != 5) ? ":" : "");
        }
        server.printP(tdE);
        server.printP(trS); P(b1) = "<td class='tdSetup' style='text-align:right' colspan='4'><button type='button' onclick=saveNet() class='save'>Save</button>  </td>  ";   server.printP(b1);   server.printP(tdE);
        server.printP(trE);
        server.printP(tableEnd);
      }

      else if (strcmp(valueFunc, "snmp") == 0) {
        P(t1) = "<table id='net' class='settingsTable'>"
                "<tr style='height:20px'>"
                "<td colspan='4' style='text-align:center;'><b>SNMP Settings</b></td>";
        server.printP(t1);
        server.printP(trS);
        server.printP(tdSe); P(rc) = "SNMP Community Read"; server.printP(rc); server.printP(tdE);
        server.printP(tdSe); server.printP(tbS); P(rcTB) = "name='readCommunity' value='"; server.printP(rcTB); server.print(netConf.readCommunity); server.printP(e0); server.printP(tdE);
        server.printP(trE);

        server.printP(trS);
        server.printP(tdSe); P(wc) = "SNMP Community Write"; server.printP(wc); server.printP(tdE);
        server.printP(tdSe); server.printP(tbS); P(wcTB) = "name='writeCommunity' value='"; server.printP(wcTB); server.print(netConf.writeCommunity); server.printP(e0); server.printP(tdE);
        server.printP(trE);

        server.printP(trS);
        server.printP(tdSe); P(snmpP) = "SNMP Port"; server.printP(snmpP); server.printP(tdE);
        server.printP(tdSe); server.printP(tbnLS); P(snmpPTB) = "name='snmpPort' style='width:130px;' value='"; server.printP(snmpPTB ); server.print(netConf.snmpPort); server.printP(e0); server.printP(tdE);
        server.printP(trE);

        server.printP(trS); P(m8) = "<td class='tdSetup' style='text-align:right' colspan='2'><button type='button' onclick=saveSNMP() class='save'>Save</button></td>"; server.printP(m8);
        server.printP(trE);
        server.printP(tableEnd);
      }

      else if (strcmp(valueFunc, "Privileges") == 0) {
        P(t1) = "<table id='net' class='settingsTable'>\n"
                "<tr style='height:20px'>\n"
                "<td colspan='3' style='text-align:center;'><b>Privileges Settings</b></td>\n"
                "</tr>\n"
                "<tr>\n"
                "<td></td><td colspan='' style='text-align:center;'>Username</td><td colspan='' style='text-align:center;'>Password</td>"
                "</tr>";
        server.printP(t1);
        server.printP(trS);
        server.printP(tdS); P(un) = "User"; server.printP(un); server.printP(tdE);
        P(up) = ("<input class='text-box' type='text' title='A-Z, a-z, 0-9' placeholder='Enter Here' ");
        server.printP(tdSe); server.printP(up); P(unU) = "name='unU' value='"; server.printP(unU); server.print(netConf.usernameU); server.printP(e0); server.printP(tdE);
        server.printP(tdSe); server.printP(up); P(pwU) = "name='pwU' value='"; server.printP(pwU); server.print(netConf.passwordU); server.printP(e0); server.printP(tdE);
        server.printP(trE);

        server.printP(trS);
        server.printP(tdSe); P(pw) = "Admin"; server.printP(pw); server.printP(tdE);

        server.printP(tdSe); server.printP(up); P(unA) = "name='unA' value='"; server.printP(unA); server.print(netConf.usernameA); server.printP(e0); server.printP(tdE);
        server.printP(tdSe); server.printP(up); P(pwA) = "name='pwA' value='"; server.printP(pwA); server.print(netConf.passwordA); server.printP(e0); server.printP(tdE);
        server.printP(trE);
        /*
                server.printP(tdS); P(rpw) = "Re-Password"; server.printP(rpw); server.printP(tdE);
                server.printP(tdS); server.printP(tbS); P(rpwA) = "name='rpwA' value='"; server.printP(rpwA); server.print(netConf.passwordA); server.printP(e0); server.printP(tdE);
                server.printP(tdS); server.printP(tbS); P(rpwU) = "name='rpwU' value='"; server.printP(rpwU); server.print(netConf.passwordU); server.printP(e0); server.printP(tdE);
                server.printP(trE);
        */
        server.printP(trS); P(m8) = "<td class='td' style='text-align:right' colspan='3'><button type='button' onclick=savePrivileges() class='save'>Save</button></td>"; server.printP(m8);
        server.printP(trE);
        server.printP(tableEnd);
      }

      else if (strcmp(valueFunc, "factory") == 0) {
        if (user == FACTORY) {
          P(t1) = "<table id='net' class='settingsTable'>"
                  "<tr style='height:20px'>"
                  "<td colspan='4' style='text-align:center;'><b>Factory Settings</b></td>";
          server.printP(t1);

          server.printP(trS);
          server.printP(tdSe); P(srto) = "System reset Timout"; server.printP(srto); server.printP(tdE);
          server.printP(tdSe); server.printP(tbS);  server.printP(Name); P(sysRSTto) = "srto"; server.printP(sysRSTto); server.printP(tbM);  server.print(sys.srto); server.printP(e0); server.printP(tdE);
          server.printP(trE);

          server.printP(trS);
          server.printP(tdSe); P(maxOW) = "Max oneWire"; server.printP(maxOW); server.printP(tdE);
          server.printP(tdSe); server.printP(tbS);  server.printP(Name); P(maxOneWire) = "maxOneWire"; server.printP(maxOneWire); server.printP(tbM);  server.print(sys.maxAlowedOneWire); server.printP(e0); server.printP(tdE);
          server.printP(trE);

          server.printP(trS);
          server.printP(tdSe); P(maxAN) = "Max Analog"; server.printP(maxAN); server.printP(tdE);
          server.printP(tdSe); server.printP(tbS);  server.printP(Name); P(maxAnalog) = "maxAnalog"; server.printP(maxAnalog); server.printP(tbM);  server.print(sys.maxAlowedAnalog); server.printP(e0); server.printP(tdE);
          server.printP(trE);
          /*
                    server.printP(trS);
                    server.printP(tdSe); P(maxHU) = "Max Humid"; server.printP(maxHU); server.printP(tdE);
                    server.printP(tdSe); server.printP(tbS);  server.printP(Name); P(maxHumid) = "maxHumid"; server.printP(maxHumid); server.printP(tbM);  server.print(sys.maxAlowedHumid); server.printP(e0); server.printP(tdE);
                    server.printP(trE);
          */
          server.printP(trS);
          server.printP(tdSe); P(maxDG) = "Max Digial"; server.printP(maxDG); server.printP(tdE);
          server.printP(tdSe); server.printP(tbS);  server.printP(Name); P(maxDigital) = "maxDigital"; server.printP(maxDigital); server.printP(tbM);  server.print(sys.maxAlowedDigital); server.printP(e0); server.printP(tdE);
          server.printP(trE);

          server.printP(trS);
          server.printP(tdSe); P(maxRE) = "Max Relay"; server.printP(maxRE); server.printP(tdE);
          server.printP(tdSe); server.printP(tbS);  server.printP(Name); P(maxRelay) = "maxRelay"; server.printP(maxRelay); server.printP(tbM);  server.print(sys.maxAlowedRelay); server.printP(e0); server.printP(tdE);
          server.printP(trE);

          server.printP(trS);
          server.printP(tdSe); P(maxRU) = "Max Rules"; server.printP(maxRU); server.printP(tdE);
          server.printP(tdSe); server.printP(tbS);  server.printP(Name); P(maxRules) = "maxRules"; server.printP(maxRules); server.printP(tbM);  server.print(sys.maxAlowedRules); server.printP(e0); server.printP(tdE);
          server.printP(trE);

          server.printP(trS);
          server.printP(tdSe); P(mac) = "MAC Address"; server.printP(mac); server.printP(tdE);

          server.printP(tdSe); P(tbMac) = ("<input type='text' class='text-box' maxlength='17' "); server.printP(tbMac);  server.printP(Name); P(macA) = "macA"; server.printP(macA); server.printP(tbM);
          for (j = 0; j < 6; j++) {
            if (sys.mac[j] < 10) server.print("0");
            server.print(sys.mac[j], HEX);
          } server.printP(e0);
          server.printP(tdE);
          server.printP(trE);

          server.printP(trS); P(m8) = "<td class='td' style='text-align:right' colspan='3'><button type='button' onclick=saveFactory() class='save'>Save</button></td>"; server.printP(m8);
          server.printP(trE);

          server.printP(tableEnd);
        }
        else {
        }
      }

      else if (strcmp(valueFunc, "syncTime") == 0) {
        setTime(syncTime());
      }

      else if (strcmp(valueFunc, "saveSettings") == 0) {
        do {
          repeat = server.readPOSTparam(valueFunc, 17, valueNum, 17);
          //***********************************
          //Time & Date Settings
          //***********************************
          if (strcmp(valueFunc, "saveClock") == 0) {
            parseTime(valueNum, CLOCK);
          }
          else if (strcmp(valueFunc, "timeserver") == 0) {
            setIP(TIME_SERVER , valueNum);
          }
          else if (strcmp(valueFunc, "ntpSyncInterval") == 0) {
            netConf.NTPSyncInterval = atoi(valueNum);
            setSyncInterval(netConf.NTPSyncInterval * 60); //sync interval in seconds
          }
          else if (strcmp(valueFunc, "timezone") == 0) {
            netConf.timeZone = atoi(valueNum);
          }
          //**********************************
          //Network Settings
          //**********************************
          else if (strcmp(valueFunc, "dhcp") == 0) {
            //   Serial.print("dhcp pre:"); Serial.println(netConf.dhcp);
            if (atoi(valueNum) == 1) {
              netConf.dhcp = true;
            } else if (atoi(valueNum) == 0) {
              netConf.dhcp = false;
            }
            //        Serial.print("dhcp after:"); Serial.println(netConf.dhcp);
          }
          else if (strcmp(valueFunc, "ip") == 0)  {
            //         Serial.print("ip pre:"); Serial.println(netConf.ip);
            setIP(IP_ADD ,  valueNum);
            //          Serial.print("ip after:"); Serial.println(netConf.ip);
          }
          else if (strcmp(valueFunc, "webPort") == 0) {
            netConf.webPort = atoi(valueNum);
          }
          else if (strcmp(valueFunc, "mask") == 0)  {
            //       Serial.print("mask pre:"); Serial.println(netConf.mask);
            setIP(MASK , valueNum);
            //       Serial.print("mask after:"); Serial.println(netConf.mask);
          }
          else if (strcmp(valueFunc, "gateway") == 0) {
            //        Serial.print("gateway after:"); Serial.println(netConf.gateway);
            setIP(GATEWAY , valueNum);
            //       Serial.print("gateway after:"); Serial.println(netConf.gateway);
          }
          else if (strcmp(valueFunc, "dns") == 0) {
            //          Serial.print("dns pre:"); Serial.println(netConf.dns);
            setIP(DNS , valueNum);
            //         Serial.print("dns after:"); Serial.println(netConf.dns);
          }
          //**********************************
          //SNMP Settings
          //**********************************
          if (strcmp(valueFunc, "readCommunity") == 0)  {
            if (strlen(valueNum) < MAX_COMMUNITY_DESCRIPTION) {
              sprintf(netConf.readCommunity, valueNum );
            }
          }
          else if (strcmp(valueFunc, "writeCommunity") == 0)  {
            if (strlen(valueNum) < MAX_COMMUNITY_DESCRIPTION)
              sprintf(netConf.writeCommunity, valueNum );
            initSNMP();
          }
          else if (strcmp(valueFunc, "snmpPort") == 0)  {
            netConf.snmpPort = atoi(valueNum);
            initSNMP();
          }
        } while (repeat);
        //  EEPROM_writeAnything(netStart, netConf);
        eeSave[NET] = 1;
      }


      //**********************************
      //Privileges Settings
      //**********************************
      else if (strcmp(valueFunc, "savePrivileges") == 0) {
        do {
          repeat = server.readPOSTparam(valueFunc, 17, valueNum, 17);
          if (strcmp(valueFunc, "userName") == 0)  {
            if (strlen(valueNum) < 9)
              sprintf(netConf.usernameU, valueNum );
          }
          else if (strcmp(valueFunc, "userPass") == 0)  {
            if (strlen(valueNum) < 9)
              sprintf(netConf.passwordU, valueNum );
          }
          else if (strcmp(valueFunc, "adminName") == 0)  {
            if (strlen(valueNum) < 9)
              sprintf(netConf.usernameA, valueNum );
          }
          else if (strcmp(valueFunc, "adminPass") == 0)  {
            if (strlen(valueNum) < 9)
              sprintf(netConf.passwordA, valueNum );
          }
        } while (repeat);
        //  EEPROM_writeAnything(netStart, netConf);
        eeSave[NET] = 1;
      }

      else if (strcmp(valueFunc, "saveFactory") == 0 && user == FACTORY) {
        //        Serial.println(valueFunc);
        do {
          repeat = server.readPOSTparam(valueFunc, 17, valueNum, 17);
          Serial.println(valueFunc);
          Serial.println(valueNum);
          //***********************************
          //Factory Settings
          //***********************************
          if      (strcmp(valueFunc, "srto") == 0  && atoi(valueNum) && atoi(valueNum) > 0 ) {
            sys.srto =  atoi(valueNum);  //system reset timeout
            resetSysTimout();
          }
          else if (strcmp(valueFunc, "maxOneWire") == 0  && atoi(valueNum) < MAX_ONE_WIRE && atoi(valueNum) >= 0) sys.maxAlowedOneWire =  atoi(valueNum);
          else if (strcmp(valueFunc, "maxAnalog")  == 0  && atoi(valueNum) < MAX_ANALOG && atoi(valueNum)   >= 0) sys.maxAlowedAnalog  =  atoi(valueNum);
          else if (strcmp(valueFunc, "maxDigital") == 0  && atoi(valueNum) < MAX_DIGITAL && atoi(valueNum)  >= 0) sys.maxAlowedDigital =  atoi(valueNum);
          else if (strcmp(valueFunc, "maxRelay")   == 0  && atoi(valueNum) < MAX_RELAY && atoi(valueNum)    >= 0) sys.maxAlowedRelay   =  atoi(valueNum);
          else if (strcmp(valueFunc, "maxRules")   == 0  && atoi(valueNum) < MAX_RULES && atoi(valueNum)    >= 0) sys.maxAlowedRules   =  atoi(valueNum);
          else if (strcmp(valueFunc, "macAddress") == 0 ) setMAC(valueNum);

        } while (repeat);
        eeSave[SYS] = 1;
        eeSaveHandler(); //save all parameters before booting
        //   SWreset();
      }
      else if (strcmp(valueFunc, "reboot") == 0) {
        eeSaveHandler(); //save all parameters before booting
        SWreset();
      }
    }
  }
  else
  {
    server.httpUnauthorized(); //Unauthorized
    return;
  }
}

void logout(WebServer & server, WebServer::ConnectionType type, char *, bool) {
  server.logOut();
  if (type == WebServer::GET) { //http reqested to load a page
    //  isAdmin = false;
    user = NA;
  }
}

String getClockString() {
  //Serial.println("Clock Updated");
  String st = "";
  if (hour() < 10)
    st += ("0");
  st += hour();
  st += (":");
  if (minute() < 10)
    st += ("0");
  st += (minute());
  st += ("       ");
  if (day() < 10)
    st += ("0");
  st += day();
  st += ("/");
  if (month() < 10)
    st += ("0");
  st += month();
  st += ("/");
  st += year();
  st += ("</a>");
  return st;
}

String htmlClock() {
  String st = ("<a class='mmc' style='color:#cf0000;' id='clock'>");
  st += getClockString();
  return st;
}


#endif
