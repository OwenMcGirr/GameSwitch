#ifndef Timer_h
#define Timer_h

#include "Arduino.h"

class Timer
{
public:
  Timer();
  void updateTimer();
  void resetTimer();
  long getElapsedTime();

private:
  long startTime;
  long elapsedTime;
  boolean timerRunning;
};

#endif
