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


void setupWiFi(void);
void setupSTA(void);
void setupDisplay(void);
void setupAP(void);
time_t getNtpTime(void);
void sendNTPpacket(WiFiUDP *u);
void _displayBusy(void);
void _displayIP(void);
char displayIP(void);
void displayAP(void);
void displayDash(void);
void stopDisplayBusy(void);
void displayClock(void);
void setupTime(void);
void displayBusy(char digit);

#define SECS_PER_HOUR 3600
extern time_t timeOffset = -1;
extern unsigned long millisOffset = 0;
#define timeSet 1
#define timeNotSet 0
extern char currentStatus = timeNotSet;
#define setTime(t) (timeOffset=(t),millisOffset=millis())
#define timeStatus() (timeOffset + (millis()-millisOffset)/1000)
#define now() (millis()/1000-settings.timezone*SECS_PER_HOUR)
#define hour() (( now() / SECS_PER_HOUR ) % 24)
#define minute() (( now() / 60 ) % 60)
#define second() (( now() ) % 60)

#define setSyncProvider(provider)
#define setSyncInterval(interval)

extern char leds[] = { 16, 5, 4, 2, 14, 12, 13, 15, 3, 1 };
#define NUM_LEDS  (sizeof leds/sizeof *leds)


#endif

