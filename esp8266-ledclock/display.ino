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

volatile int value = 0;

void clear() {
  for (char i = 0 ; i < NUM_LEDS ; i++) {
    digitalWrite(leds[i], LOW);
  }
}

void display() {
  for (char i = 0 ; i < NUM_LEDS ; i++) {
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

volatile char busyValue;

void displayBusy(char digit) {
  busyValue = 1;
  ticker.attach(0.1, _displayBusy);
}

void stopDisplayBusy() {
  ticker.detach();
}

void _displayBusy() {
  busyValue = (busyValue << 1) & 0x3FF;
  if (busyValue == 0)
      busyValue = 1;
  value = busyValue;
  display();
}

// End twirler handler.

// IP Display handler.
volatile signed char dispOctet = -1;

char displayIP() {
  if (dispOctet > -1) {
    return 1;
  }
  if (digitalRead(SETUP_PIN) == 1) return 0;
  dispOctet = 0;
  ticker.attach(1.0, _displayIP);
  return 0;
}

void _displayIP() {
  if (dispOctet > 3) {
    ticker.detach();
    dispOctet = -1;
    clockMode == MODE_CLOCK ? displayClock() : displayAP();
    return;
  }
  clearDigits();
  uint8_t octet = (uint32_t(clockMode == MODE_CLOCK ? WiFi.localIP() : WiFi.softAPIP()) >> (8 * dispOctet++)) & 0xff;
  value = octet;
  display();
}

// end Ip display handler.

void displayClock() {
  int h = hour() % 12;
  int m = minute();

  value = (h << 6) | m;

  if (timeStatus() != timeSet) value = 0x303;
  
  display();
}

void setupDisplay() {
  for (char i=0; i < NUM_LEDS; i++)
    pinMode(leds[i], OUTPUT);
  displayDash();
}

