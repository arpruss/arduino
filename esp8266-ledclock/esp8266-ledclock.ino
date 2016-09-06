/*  Copyright (C) 2016 Buxtronix and Alexander Pruss

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <Ticker.h>

#include <Time.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <stdlib.h>

#include "settings.h"
#include "mainPage.h"
#include "clock.h"

ESP8266WebServer server (80);

char mode = MODE_CLOCK;
String httpUpdateResponse;

time_t prevDisplay = 0;

void setMode(char newMode) {
  DebugLn("request set mode to "+String((int)newMode));
  mode = newMode;
  prevDisplay = 0;
}

String getMode() {
  if (mode == MODE_CLOCK)
    return "clock";
  else
    return "LEDs";
}

uint16_t parseNumber(char* number) {
  if (number[0] == '0' && (number[1] == 'x' || number[1] == 'X')) {
    return (uint16_t)strtoul(number, NULL, 16);
  }
  else if (number[0] == '0' && (number[1] == 'b' || number[1] == 'B')) {
    return (uint16_t)strtoul(number, NULL, 2);
  }
  else {
    return atoi(number);
  }
}

char handleAnimation(char* in) {
  DebugLn("Animate "+String(in));
  char* p = in;
  int argCount = 1;
  while (*p) {
    if (*p == ':') {
      argCount++;
      *p = 0;
    }
    p++;
  }
  if (argCount < 4)
    return 0;
  char returnToClock = atoi(in);  
  in += strlen(in) + 1;
  argCount--;
  float delayTime = atof(in);
  if (delayTime == 0)
    return 0;
  in += strlen(in) + 1;
  argCount--;
  int repeat = atoi(in);
  in += strlen(in) + 1;
  argCount--;

  if (argCount > MAX_ANIMATE_COUNT)
    argCount = MAX_ANIMATE_COUNT;
  
  uint16_t data[argCount];
  for (int i = 0 ; i < argCount ; i++) {
    data[i] = parseNumber(in);
    DebugLn(String(data[i]));
    in += strlen(in) + 1;
  }
  animate(returnToClock, delayTime, repeat, argCount, data);
  return 1;
}

void handleIO() {
  DebugLn("handleIO in mode "+String((int)mode));

  String value = server.arg("value");
  int valueLen = value.length();
  char caValue[valueLen+1];
  value.toCharArray(caValue,valueLen+1);
  String op = server.arg("op");
  String status = "";
  String success = String("\nresult: success with ")+op+String("\n");

  DebugLn("operation "+op);

  if (op == "clock") {
    setMode(MODE_CLOCK);
    clearDisplay();
    status = success;
  }
  else if (op == "set") {
    setMode(MODE_LEDS);
    displayRaw(parseNumber(caValue));
    status = success;
  }
  else if (op == "or") {
    setMode(MODE_LEDS);
    displayRaw(getDisplayRaw() | parseNumber(caValue));
    status = success;
  }
  else if (op == "andnot") {
    setMode(MODE_LEDS);
    displayRaw(getDisplayRaw() & ~parseNumber(caValue));
    status = success;
  }
  else if (op == "animate") {
    setMode(MODE_LEDS);
    if (handleAnimation(caValue))
      status = success;
  }
  else if (op == "tone") {
    status = success;
  }
  
  server.send(200, "text/plain", 
    String("mode: ")+
    String(getMode())+
    String("\nleds: "     )+
    String(getDisplayRaw())+
    String("\nbutton: ")+
    String(!digitalRead(SETUP_PIN))+
    status);

  if (op == "tone") {
    play(caValue);
  }
}

void handleRoot() {
  DebugLn("handleRoot");
  String s = MAIN_page;
  time_t t = now();
  s.replace("@@SSID@@", settings.ssid);
  s.replace("@@PSK@@", settings.psk);
  s.replace("@@TZ@@", String(settings.timezone));
  s.replace("@@USDST@@", settings.usdst?"checked":"");
  s.replace("@@HOUR@@", String(adjustedHour(t)));
  s.replace("@@MIN@@", String(minute(t)));
  s.replace("@@NTPSRV@@", settings.timeserver);
  s.replace("@@NTPINT@@", String(settings.interval));
  s.replace("@@SYNCSTATUS@@", timeStatus() == timeSet ? "OK" : "Overdue");
  s.replace("@@CLOCKNAME@@", settings.name);
  s.replace("@@UPDATERESPONSE@@", httpUpdateResponse);
  s.replace("@@MODE@@", getMode());
  httpUpdateResponse = "";
  server.send(200, "text/html", s);
}

void handleForm() {
  DebugLn("handleForm");
  DebugLn("mode "+String(WiFi.status()));
  String update_wifi = server.arg("update_wifi");
  String t_ssid = server.arg("ssid");
  String t_psk = server.arg("psk");
  String t_timeserver = server.arg("ntpsrv");
  t_timeserver.toCharArray(settings.timeserver, EEPROM_TIMESERVER_LENGTH, 0);
  if (update_wifi == "1") {
    settings.ssid = t_ssid;
    settings.psk = t_psk;
  }
  String tz = server.arg("timezone");

  if (tz.length()) {
    settings.timezone = tz.toInt();
  }

  String usdst = server.arg("usdst");
  settings.usdst = (usdst == "1");

  time_t newTime = getNtpTime();
  if (newTime) {
    setTime(newTime);
  }
  String syncInt = server.arg("ntpint");
  settings.interval = syncInt.toInt();

  settings.name = server.arg("clockname");
  settings.name.replace("+", " ");

  httpUpdateResponse = "The configuration was updated.";

  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Moved");

  settings.Save();
  
  if (update_wifi == "1") {
    delay(500);
    setupWiFi(0);
  }
}

void setup() {
  DebugStart();
  setupDisplay();
  setupWiFi(1);
  setupTime();
  server.on("/", handleRoot);
  server.on("/form", handleForm);
  server.on("/io", handleIO);
  server.begin();
}

void loop() {
  static unsigned long lastSetupPin = 0;
  server.handleClient();
  if (mode == MODE_CLOCK) {
    if (!digitalRead(SETUP_PIN)) {
      if (lastSetupPin + 1000 <= millis()) {
        lastSetupPin = millis();
        uint16_t data[5];
        IPAddress addr = WiFi.localIP();
        data[0] = 0;
        data[1] = (0 << 8)|(addr & 0xff);
        data[2] = (0 << 8)|(addr & 0xff);
        data[3] = (0 << 8)|(addr & 0xff);
        data[4] = (1 << 8)|((addr>>8) & 0xff);
        data[5] = (1 << 8)|((addr>>8) & 0xff);
        data[6] = (1 << 8)|((addr>>8) & 0xff);
        data[7] = (2 << 8)|((addr>>16) & 0xff);
        data[8] = (2 << 8)|((addr>>16) & 0xff);
        data[9] = (2 << 8)|((addr>>16) & 0xff);
        data[10] = (3 << 8)|((addr>>24) & 0xff);
        data[11] = (3 << 8)|((addr>>24) & 0xff);
        data[12] = (3 << 8)|((addr>>24) & 0xff);
        data[13] = 0;
        setMode(MODE_LEDS);
        animate(1, 2.0, 1, 14, data);
      }
      else {
        lastSetupPin = millis();
      }
    }
    if (timeStatus() != timeNotSet) {
      time_t t = now();
      if (t != prevDisplay) { //update the display only if time has changed
        prevDisplay = t;
        displayClock(t);
      }
    }
  }
  else {
    prevDisplay = -1;
  }
}

void setupWiFi(char checkAPMode) {
  settings.Load();
  if (checkAPMode) {
    // Wait up to 5s for SETUP_PIN to go low to enter AP/setup mode.
    pinMode(SETUP_PIN, INPUT);
    digitalWrite(SETUP_PIN, HIGH);
    displayAPWait();
    long start = millis();
    DebugLn("Started at "+String(start));
    while (millis() < start + 5000) {
      if (!digitalRead(SETUP_PIN) || !settings.ssid.length()) {
        DebugLn("Setting up AP");
        stopAnimation();
        setupAP();
        DebugLn("Done with AP");
        return;
      }
      delay(50);
    }
    stopAnimation();
  }
  setupSTA();
}

void setupSTA()
{
  char ssid[32];
  char psk[64];

  memset(ssid, 0, 32);
  memset(psk, 0, 64);
  displayBusy();

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  settings.ssid.toCharArray(ssid, 32);
  settings.psk.toCharArray(psk, 64);
  if (settings.psk.length()) {
    WiFi.begin(ssid, psk);
  } else {
    WiFi.begin(ssid);
  }
  
  DebugLn("Connecting to "+String(ssid));
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Debug(String(WiFi.status()));
  }
  DebugLn("Connected");
  
  stopAnimation();
  ntpActive = 1;
}

void setupAP() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(WIFI_AP_NAME);
  displayIP();
}

