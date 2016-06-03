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

#ifndef _CLOCK_H
#define _CLOCK_H

#undef DEBUG

void setupWiFi(void);
void setupSTA(void);
void setupDisplay(void);
void setupAP(void);
time_t getNtpTime(void);
void sendNTPpacket(WiFiUDP *u);
void _displayBusy(void);
void _displayIP(void);
void displayIP(void);
void displayDash(void);
void stopDisplayBusy(void);
void displayClock(void);
void setupTime(void);
void displayBusy(void);
uint8_t adjustedHour(time_t t);

#ifndef DEBUG
extern const char leds[] = { 16, 5, 4, 2, 14, 12, 13, 15, 3, 1 };
#define DebugStart()
#define DebugLn(s)
#define Debug(s)

#else
extern const char leds[] = { 16, 5, 4, 2, 14, 12, 13, 15 };
#define DebugStart() Serial.begin(115200)
#define DebugLn(s) Serial.println((s))
#define Debug(s) Serial.print((s))
#endif

#define NUM_LEDS  (sizeof leds / sizeof *leds)
#define SETUP_PIN 0

#endif

