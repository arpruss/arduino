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

static volatile unsigned int value = 0;

void clear() {
  for (char i = 0 ; i < NUM_LEDS ; i++) {
      digitalWrite(leds[i], LOW);
  }
}

void display() {
  for (int i = 0 ; i < NUM_LEDS ; i++) {
    if (value & (1<<i))
      digitalWrite(leds[i], HIGH);
    else 
      digitalWrite(leds[i], LOW);
  }
}

void displayAP() {
  value = 0xFFFF;
  display();
}

void displayDash() {
  value = 0x555;
  display();
}

void clearDigits() {
  value = 0;
  display();
}

// Twirler handler.
Ticker ticker;

volatile unsigned int busyValue;
volatile char busyInc;

void displayBusy() {
  busyValue = 1;
  busyInc = 1;
  ticker.attach(0.1, _displayBusy);
}

void stopDisplayBusy() {
  ticker.detach();
  clear();
}

void stopDisplayAP() {
  ticker.detach();
  clear();
}

void _displayBusy() {
  if (busyInc) {
    busyValue = (busyValue << 1) & 0x3FF;
    if (busyValue == 0) {
      busyValue = 0x100;
      busyInc = 0;
    }
  }
  else {
    busyValue >>= 1;
    if (busyValue == 0) {
      busyValue = 2;
      busyInc = 1;
    }
  }
  value = busyValue;
  display();
}

// End twirler handler.

// IP Display handler.
static volatile uint16_t dispOctet = 0;

void displayIP() {
  dispOctet = 0;
  ticker.attach(4.0, _displayIP);
}

void _displayIP() {
  if (dispOctet > 3) {
    dispOctet = 0;
  }

  uint8_t octet = ( WiFi.softAPIP() >> (8 * dispOctet)) & 0xff;
  value = octet | (dispOctet<<8);
  dispOctet++;
  display();
}

// end Ip display handler.

void displayClock() {
  if (timeStatus() != timeSet) {
    value = 0x303;
  }
  else {
    time_t n = now();
    int h = adjustedHour(n) % 12;
    if (h == 0)
      h = 12;
    int m = minute(n);

    value = (h << 6) | m;
  }

  
  display();
}

void setupDisplay() {
  for (char i=0; i < NUM_LEDS; i++)
     pinMode(leds[i], OUTPUT);
  displayDash();
}

