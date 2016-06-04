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

//#define MODE_SETUP    0

ESP8266WebServer server (80);

char mode = MODE_CLOCK;
String httpUpdateResponse;

time_t prevDisplay = 0;

void setMode(int newMode) {
  mode = newMode;
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
  else {
    return atoi(number);
  }
}

char handleAnimation(char* in) {
  char* p = in;
  int argCount = 1;
  while (*p) 
    if (*p == ',') {
      argCount++;
      *p = 0;
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
  uint16_t data[argCount];
  for (int i = 0 ; i < argCount ; i++) {
    data[i] = parseNumber(in);
    in += strlen(in) + 1;
  }
  animate(returnToClock, delayTime, repeat, argCount, data);
  return 1;
}

void handleLEDs() {
  String value = server.arg("value");
  int valueLen = value.length();
  char caValue[valueLen+1];
  value.toCharArray(caValue,valueLen+1);
  String op = server.arg("op");
  String status = "";
  static const String success = String("\nSuccess\n");
  if (op == "clock") {
    mode = MODE_CLOCK;
    clearDisplay();
    status = success;
  }
  else if (op == "set") {
    mode = MODE_LEDS;
    displayRaw(parseNumber(caValue));
    status = success;
  }
  else if (op == "or") {
    displayRaw(getDisplayRaw() | parseNumber(caValue));
    status = success;
  }
  else if (op == "andnot") {
    displayRaw(getDisplayRaw() & ~parseNumber(caValue));
    status = success;
  }
  else if (op == "animate") {
    if (handleAnimation(caValue))
      status = success;
  }
  server.send(200, "text/plain", 
    String("mode: ")+
    String((mode==MODE_CLOCK)?"clock":"led")+
    String("\nleds: ")+
    String(getDisplayRaw())+
    success);
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

void handleLED() {
  String value = server.arg("value");
  String op = server.arg("op");
  char success = 0;
  if (op == "clock") {
    mode = MODE_CLOCK;
    clearDisplay();
    success = 1;
  }
  else if (op == "set") {
    mode = MODE_LEDS;
    displayRaw(value.toInt());
    success = 1;
  }
  else if (op == "or") {
    displayRaw(getDisplayRaw() | value.toInt());
    success = 1;
  }
  else if (op == "andnot") {
    displayRaw(getDisplayRaw() & ~value.toInt());
    success = 1;
  }
  server.send(200, "text/plain", String("mode: ")+
    getMode()+
    String("\nleds: ")+String(getDisplayRaw())+
    String(success ? "\n\nSuccess!\n" : "\n" ) );
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
  server.on("/led", handleLED);
  server.begin();
}

void loop() {
  static unsigned long lastSetupPin = 0;
  server.handleClient();
  if (!digitalRead(SETUP_PIN)) {
    if (lastSetupPin + 1000 < millis()) {
      uint16_t data[5];
      IPAddress addr = WiFi.localIP();
      data[0] = (1 << 8)|(addr & 0xff);
      data[1] = (2 << 8)|((addr>>8) & 0xff);
      data[2] = (3 << 8)|((addr>>16) & 0xff);
      data[3] = (4 << 8)|((addr>>24) & 0xff);
      data[4] = 0;
      mode = MODE_LEDS;
      animate(1, 6.0, 1, 5, data);
    }
    lastSetupPin = millis();
  }
  if (mode == MODE_CLOCK) {
    if (timeStatus() != timeNotSet) {
      if (now() != prevDisplay) { //update the display only if time has changed
        prevDisplay = now();
        displayClock();
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
    digitalWrite(SETUP_PIN, LOW);
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

