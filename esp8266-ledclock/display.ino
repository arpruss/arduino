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

static volatile uint16_t value = 0;
static uint16_t busyAnimation[] = { 
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200,
  0x100, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

static uint16_t apWaitAnimation[] = { 
  0x201, 0x102, 0x84, 0x48, 0x20|0x10, 0x48, 0x84, 0x102
};

// Twirler handler.
static Ticker ticker;

static volatile char animateRepeat;
static volatile int animateCount;
static volatile uint16_t* animateValues;
static volatile int animatePos;
static volatile char animateReturnToClock;

void _display() {
  for (int i = 0 ; i < NUM_LEDS ; i++) {
    if (value & (1<<i))
      digitalWrite(leds[i], HIGH);
    else 
      digitalWrite(leds[i], LOW);
  }
}

void displayStatic() {
  ticker.detach();
  _display();
}

void clearDisplay() {
  value = 0;
  displayStatic();
}

void clearDigits() {
  value = 0;
  displayStatic();
}

void displayRaw(uint16_t v) {
  value = v;
  displayStatic();
}

uint16_t getDisplayRaw() {
  return value;
}

void _animate(void) {
  if (animatePos >= animateCount) {
    
    if (animateRepeat != FOREVER)
      animateRepeat--;

    if (animateRepeat != 0) {
      animatePos = 0;
    }
    else {
      ticker.detach();
      if (animateReturnToClock)
        setMode(MODE_CLOCK);
      return;
    }
  }
  value = animateValues[animatePos];
  _display();
  animatePos++;
}

void animate(char returnToClock, float delayTime, int repeatCount, int count, uint16_t* values) {
  animateRepeat = repeatCount;
  animateCount = count;
  animateValues = values;
  animateReturnToClock = returnToClock;
  animatePos = 0;
  ticker.attach(delayTime, _animate);
}

void stopAnimation(void) {
  ticker.detach();
  clearDisplay();
}

void displayAPWait(void) {
  animate(0, 0.1, FOREVER, sizeof(apWaitAnimation)/sizeof(*apWaitAnimation), apWaitAnimation);
}

void displayBusy(void) {
  animate(0, 0.1, FOREVER, sizeof(busyAnimation)/sizeof(*busyAnimation), busyAnimation);
}

void clearAnimation() {
  ticker.detach();
  clearDisplay();
}

// IP Display handler.
static volatile uint16_t dispOctet = 0;

void displayIP() {
  dispOctet = 0;
  ticker.attach(6.0, _displayIP);
}

void _displayIP() {
  if (dispOctet > 3) {
    dispOctet = 0;
  }

  uint8_t octet = ( WiFi.softAPIP() >> (8 * dispOctet)) & 0xff;
  value = octet | (dispOctet<<8);
  dispOctet++;
  _display();
}

void displayClock() {
  if (timeStatus() != timeSet) {
    value = 0x3FF;
  }
  else {
    time_t n = now();
    int h = adjustedHour(n) % 12;
    if (h == 0)
      h = 12;
    int m = minute(n);

    value = (h << 6) | m;
  }
  
  displayStatic();
}

void setupDisplay() {
  for (char i=0; i < NUM_LEDS; i++)
     pinMode(leds[i], OUTPUT);
  clearDisplay();
}

