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
#define CONNECTION_SINK

#ifdef CONNECTION_SINK
# define LED_ON LOW
# define LED_OFF HIGH
#else
# define LED_ON HIGH
# define LED_OFF LOW
#endif

#define MODE_CLOCK    1
#define MODE_LEDS     2
#define FOREVER -1

#define MAX_ANIMATE_COUNT 32

void setMode(char mode);
void animate(char returnToClock, float delayTime, int repeatCount, int count, uint16_t* values);
void setupWiFi(char checkAPMode);
void setupSTA(void);
void setupDisplay(void);
void setupAP(void);
time_t getNtpTime(void);
void sendNTPpacket(WiFiUDP *u);
void _displayIP(void);
void displayIP(void);
void displayClock(time_t t);
void clearDisplay(void);
void stopAnimation(void);
void setupTime(void);
void displayBusy(void);
void displayAPWait(void);
uint8_t adjustedHour(time_t t);
uint16_t getDisplayRaw(void);
void displayRaw(uint16_t v);
extern char ntpActive = 0;

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

