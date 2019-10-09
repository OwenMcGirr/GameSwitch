#ifndef XboxManager_h
#define XboxManager_h

#include "Arduino.h"
#include "Joystick.h"

#define DOWN_UP_DELAY 100

#define AXIS_UP_LEFT -127
#define AXIS_DOWN_RIGHT 127
#define AXIS_MIDDLE 0

#define X1_BUTTON 0
#define X2_BUTTON 1
#define LEFT_TRIGGER_BUTTON 2
#define LEFT_BUMPER_BUTTON 3
#define A_BUTTON 4
#define B_BUTTON 5
#define VIEW_BUTTON 6
#define MENU_BUTTON 7

class XboxManager
{
  public:
    XboxManager();
    void begin();
    void buttonDownUp(int b);
    void buttonDown(int b);
    void buttonUp(int b);
    void setXAxis(int x);
    void setYAxis(int y);
    void reset();
  private:
};

#endif
