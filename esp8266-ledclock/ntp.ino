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

#include "clock.h"

unsigned int localPort = 4097;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];
byte sendBuffer[] = {
  0b11100011,          // LI, Version, Mode.
  0x0,                 // Stratum unspecified.
  0x6,                 // Polling interval
  0xEC,                // Clock precision.
  0x0, 0x0, 0x0, 0x0}; // Reference ...

void setupTime() {
  setSyncProvider(getNtpTime);
  setSyncInterval(settings.interval);
}
  

time_t getNtpTime()
{
  if (!ntpActive)
    return 0;
  DebugLn("getNtpTime");
  WiFiUDP udp;
  udp.begin(localPort);
  while (udp.parsePacket() > 0) ; // discard any previously received packets
  for (int i = 0 ; i < 5 ; i++) { // 5 retries.
    DebugLn("send packet");
    sendNTPpacket(&udp);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
      if (udp.parsePacket()) {
         udp.read(packetBuffer, NTP_PACKET_SIZE);
         // Extract seconds portion.
         unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
         unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
         unsigned long secSince1900 = highWord << 16 | lowWord;
         udp.flush();
         time_t standardTime = secSince1900 - 2208988800UL + settings.timezone * 60;
         return standardTime;
      }
      delay(10);
    }
  }
  DebugLn("failed");
  return 0; // return 0 if unable to get the time
}

// adjust hour for DST if needed
// US+Canada only
// Political message: Let's abolish DST, or move to DST all year round!
uint8_t adjustedHour(time_t t) {
  if (!settings.usdst || t == 0)
    return hour(t);
  uint8_t m = month(t);
  uint8_t h = hour(t);
  char dst = 0;
  if (m == 3) {
    // check for March spring-forward
    // spring forward at 3 am second Sunday in March
    int8_t w = weekday(t);
    int8_t d = day(t);
    if (w == 0) {
      // it's Sunday
      if (d > 14) {
        dst = 1; // three or more Sundays have passed
      }
      else if (d > 7) {
        // it's that pesky second Sunday
        
        if (h >= 3)
          dst = 1; // after 3 am ST
      }
    }
    else if (d - w > 7) {
      dst = 1; // two or more Sundays have passed
    }
  } else if (m == 11) {
    // check for November fall-back
    dst = 1;
    int8_t w = weekday(t);
    int8_t d = day(t);
    if (w == 0) {
      if (d > 7) {
        dst = 0; // two or more Sundays have passed
      }
      else {
        // that first Sunday
        if (h >= 1)
          dst = 0; // after 1 am ST
      }
    } else if (d - w >= 1) {
      // it's after the first Sunday
      dst = 0;
    }
  }
  else if (3 < m && m < 11) {
    dst = 1;
  }
  return dst ? (1+h)%24 : h;
}

void sendNTPpacket(WiFiUDP *u) {
  // Zeroise the buffer.
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  memcpy(packetBuffer, sendBuffer, 16);

  if (u->beginPacket(settings.timeserver, 123)) {
    u->write(packetBuffer, NTP_PACKET_SIZE);
    u->endPacket();
  }
}
