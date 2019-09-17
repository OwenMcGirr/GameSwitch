#ifndef XboxManager_h
#define XboxManager_h

#include "Arduino.h"
#include "Joystick.h"

#define DOWN_UP_DELAY 250

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
    void keyDownUp(int k);
    void keyDown(int k);
    void keyUp(int k);
    void reset();
  private:
};

#endif
