#ifndef InputSwitch_h
#define InputSwitch_h

#include "Arduino.h"
#include "Timer.h"

class InputSwitch
{
public:
  InputSwitch();
  void setCurrentState(boolean current);
  void setPreviousState();
  void updateHoldTime();
  void checkShouldResetHoldTime();
  long getHoldTime();
  boolean isDown();
  boolean wasJustReleased();
  boolean wasJustPressed();

private:
  int pin;
  boolean currentState;
  boolean previousState;
  Timer holdTimer;
};

#endif
