#ifndef XboxManager_h
#define XboxManager_h

#include "Arduino.h"
#include "Joystick.h"

#define DOWN_UP_DELAY 100

#define AXIS_UP_LEFT -127
#define AXIS_DOWN_RIGHT 127
#define AXIS_MIDDLE 0

#define VIEW_BUTTON 0
#define MENU_BUTTON 1
#define RIGHT_TRIGGER_BUTTON 2
#define RIGHT_BUMPER_BUTTON 3
#define X_BUTTON 4
#define Y_BUTTON 5
#define X1_BUTTON 6
#define X2_BUTTON 7

class XboxManager
{
public:
  XboxManager();
  void begin();
  void buttonDownUp(int b);
  void buttonDown(int b);
  void buttonUp(int b);
  void reset();

private:
  void setXAxis(int x);
  void setYAxis(int y);
};

#endif
