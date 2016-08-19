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

  char* start = in;
  for (int rep = 0 ; rep < repeats ; rep++) {
    in = start;
    
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
  pinMode(SETUP_PIN, INPUT);
  digitalWrite(SETUP_PIN, HIGH);
}

