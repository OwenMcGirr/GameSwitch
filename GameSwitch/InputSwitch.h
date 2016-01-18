#ifndef InputSwitch_h
#define InputSwitch_h

#include "Arduino.h"
#include "Timer.h"

#define DEBOUNCE_DELAY 10

class InputSwitch
{
  public:
    InputSwitch(int p);
    void debounce();
    void setPreviousState();
    void updateHoldTime();
    void checkShouldResetHoldTime();
    long getHoldTime();
    boolean isDown();
    boolean wasJustReleased();
  private:
    int pin;
    boolean currentState;
    boolean previousState;
    Timer holdTimer;
};

#endif
