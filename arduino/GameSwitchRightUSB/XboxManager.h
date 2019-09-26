#ifndef XboxManager_h
#define XboxManager_h

#include "Arduino.h"
#include "Joystick.h"

#define DOWN_UP_DELAY 250

#define VIEW_BUTTON 0
#define MENU_BUTTON 1
#define RA_BUTTON 2
#define RB_BUTTON 3
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
};

#endif
