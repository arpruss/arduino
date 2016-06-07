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

char play(char *in) {
  char* p = in;
  int argCount = 1;
  while (*p) {
    if (*p == ':') {
      argCount++;
      *p = 0;
    }
    p++;
  }

  if (argCount < 3 || argCount % 2 == 0)
    return 0;

  int repeats = atoi(in);
  in += strlen(in) + 1;
  argCount--;

  int toneCount = argCount / 2;

  if (toneCount > MAX_TONES)
    toneCount = MAX_TONES;

  for (int rep = 0 ; rep < repeats ; rep++) {
    char* start = in;
    
    for (int i = 0 ; i < toneCount ; i++) {
      uint16_t duration = atoi(in);
      in += strlen(in) + 1;
      uint16_t freq = atoi(in);
      in += strlen(in) + 1;

      unsigned long startTime = millis();

      if (freq)
        tone(TONE_PIN, freq);
      else
        noTone(TONE_PIN);

      yield();

      unsigned long curTime = millis();

      if (curTime < startTime + duration)
        delay(startTime + duration - curTime);
    }
  }
  noTone(TONE_PIN);
}

