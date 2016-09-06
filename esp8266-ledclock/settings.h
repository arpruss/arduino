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

#include <EEPROM.h>

#ifndef SETTINGS_H
#define SETTINGS_H

#define EEPROM_WIFI_SIZE 512
#define EEPROM_MAGIC "NtPc"
#define EEPROM_MAGIC_OFFSET 0
#define EEPROM_MAGIC_LENGTH 4
#define EEPROM_SSID_OFFSET EEPROM_MAGIC_OFFSET + EEPROM_MAGIC_LENGTH
#define EEPROM_SSID_LENGTH 32
#define EEPROM_PSK_OFFSET EEPROM_SSID_OFFSET + EEPROM_SSID_LENGTH
#define EEPROM_PSK_LENGTH 64
#define EEPROM_TZ_OFFSET EEPROM_PSK_OFFSET + EEPROM_PSK_LENGTH
#define EEPROM_TZ_LENGTH 2
#define EEPROM_USDST_OFFSET EEPROM_TZ_OFFSET + EEPROM_TZ_LENGTH
#define EEPROM_USDST_LENGTH 1
#define EEPROM_TIMESERVER_OFFSET EEPROM_USDST_OFFSET + EEPROM_USDST_LENGTH
#define EEPROM_TIMESERVER_LENGTH 32
#define EEPROM_INTERVAL_OFFSET EEPROM_TIMESERVER_OFFSET + EEPROM_TIMESERVER_LENGTH
#define EEPROM_INTERVAL_LENGTH 2
#define EEPROM_NAME_OFFSET EEPROM_INTERVAL_OFFSET + EEPROM_INTERVAL_LENGTH
#define EEPROM_NAME_LENGTH 32

#define DEFAULT_TIMESERVER "time.nist.gov"
#define MINIMUM_INTERVAL 60
#define DEFAULT_INTERVAL (60*60)
#define DEFAULT_TIMEZONE 360

#define SSID_LENGTH EEPROM_SSID_LENGTH
#define PSK_LENGTH EEPROM_PSK_LENGTH

//#include "c:/users/alexander/Documents/Arduino/private-default.h"
char my_default_ssid[SSID_LENGTH] = "";
char my_default_psk[PSK_LENGTH] = "";

#define CLOCK_NAME "ESP-CLOCK"
#define WIFI_AP_NAME CLOCK_NAME

class Settings {

  public:
    Settings() {};

    void Load() {
      char buffer[EEPROM_WIFI_SIZE];
      EEPROM.begin(EEPROM_WIFI_SIZE);
      for (int i = 0 ; i < EEPROM_WIFI_SIZE ; i++) {
        buffer[i] = EEPROM.read(i);
      }
      EEPROM.end();

      // Verify magic;
      String magic;
      for (int i = EEPROM_MAGIC_OFFSET ; i < EEPROM_MAGIC_OFFSET+EEPROM_MAGIC_LENGTH ; i++) {
        magic += buffer[i];
      }
      if (magic != EEPROM_MAGIC) {
        interval = DEFAULT_INTERVAL;
        timezone = 360;
        strcpy(timeserver, DEFAULT_TIMESERVER);
        usdst = 1;
        ssid = String(my_default_ssid);
        psk = String(my_default_psk);
        return;
      }
      // Read SSID
      ssid = "";
      for (int i = EEPROM_SSID_OFFSET ; i < EEPROM_SSID_OFFSET+EEPROM_SSID_LENGTH ; i++) {
        if (buffer[i]) ssid += buffer[i];
      }
      // Read PSK
      psk = "";
      for (int i = EEPROM_PSK_OFFSET ; i < EEPROM_PSK_OFFSET+EEPROM_PSK_LENGTH ; i++) {
        if (buffer[i]) psk += buffer[i];
      }

      timezone = int16_t(buffer[EEPROM_TZ_OFFSET]) << 8;
      timezone |= buffer[EEPROM_TZ_OFFSET+1];

      usdst = buffer[EEPROM_USDST_OFFSET];

      strncpy(timeserver, &buffer[EEPROM_TIMESERVER_OFFSET], EEPROM_TIMESERVER_LENGTH);
      if (strlen(timeserver) < 1) {
        strcpy(timeserver, DEFAULT_TIMESERVER);
      }

      interval = time_t(buffer[EEPROM_INTERVAL_OFFSET]) << 8;
      interval |= buffer[EEPROM_INTERVAL_OFFSET+1];
      if (interval < 0) {
        interval = DEFAULT_INTERVAL;
      }
      if (interval < MINIMUM_INTERVAL) {
        interval = MINIMUM_INTERVAL;
      }
      // Clock name.
      name = "";
      for (int i = EEPROM_NAME_OFFSET ; i < EEPROM_NAME_OFFSET+EEPROM_NAME_LENGTH ; i++) {
        if (buffer[i]) name += buffer[i];
      }
    }

    void Save() {
      unsigned char buffer[EEPROM_WIFI_SIZE];
      memset(buffer, 0, EEPROM_WIFI_SIZE);

      // Copy magic to buffer;
      strncpy((char *)buffer, EEPROM_MAGIC, EEPROM_MAGIC_LENGTH);

      // Copy SSID to buffer;
      ssid.getBytes(&buffer[EEPROM_SSID_OFFSET], EEPROM_SSID_LENGTH, 0);
      // Copy PSK to buffer.
      psk.getBytes(&buffer[EEPROM_PSK_OFFSET], EEPROM_PSK_LENGTH, 0);
      // Copy timezone.
      buffer[EEPROM_TZ_OFFSET] = timezone >> 8;
      buffer[EEPROM_TZ_OFFSET+1] = timezone & 0xFF;
      buffer[EEPROM_USDST_OFFSET] = usdst;
      // Copy timeserver.
      strncpy((char *)&buffer[EEPROM_TIMESERVER_OFFSET], (char *)timeserver, EEPROM_TIMESERVER_LENGTH);
      // Copy interval.
      buffer[EEPROM_INTERVAL_OFFSET] = interval >> 8;
      buffer[EEPROM_INTERVAL_OFFSET+1] = interval & 0xff;
      // Copy clock name.
      name.getBytes(&buffer[EEPROM_NAME_OFFSET], EEPROM_NAME_LENGTH, 0);

      // Write to EEPROM.
      EEPROM.begin(EEPROM_WIFI_SIZE);
      for (int i = 0 ; i < EEPROM_WIFI_SIZE ; i++) {
        EEPROM.write(i, buffer[i]);
      }
      EEPROM.commit();
      EEPROM.end();
    }

    String ssid;
    String psk;
    int16_t timezone;
    char usdst;
    char timeserver[64];
    int interval;
    String name;
};

Settings settings;

#endif


