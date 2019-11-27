/*
 * Basic class for timing events.
 */

#include "Timer.h"

Timer::Timer()
{
  // set defaults
  startTime = 0;
  elapsedTime = 0;
  timerRunning = false;
}

void Timer::updateTimer()
{
  if (!timerRunning)
  {
    startTime = millis();
    timerRunning = true;
  }
  else
  {
    elapsedTime = millis() - startTime;
  }
}

void Timer::resetTimer()
{
  startTime = 0;
  elapsedTime = 0;
  timerRunning = false;
}

long Timer::getElapsedTime()
{
  return elapsedTime;
}
