#ifndef XboxManager_h
#define XboxManager_h

#include "Arduino.h"
#include "Joystick.h"

#define DOWN_UP_DELAY 250

#define AXIS_UP_LEFT -127
#define AXIS_DOWN_RIGHT 127
#define AXIS_MIDDLE 0

#define X1_BUTTON 0
#define X2_BUTTON 1
#define LA_BUTTON 2
#define LB_BUTTON 3
#define A_BUTTON 4
#define B_BUTTON 5
#define VIEW_BUTTON 6
#define MENU_BUTTON 7

//Defining the joystick REPORT_ID and profile type
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_JOYSTICK, 8, 0,
                   true, true, false,
                   false, false, false,
                   false, false,
                   false, false, false);

class XboxManager
{
  public:
    XboxManager();
    void buttonDownUp(int b);
    void buttonDown(int b);
    void buttonUp(int b);
    void reset();
  private:
};

#endif
