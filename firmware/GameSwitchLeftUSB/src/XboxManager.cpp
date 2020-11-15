/*
   Basic class for controlling Xbox.
*/

#include "XboxManager.h"

//Defining the joystick REPORT_ID and profile type
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_JOYSTICK, 8, 0,
                   true, true, false,
                   false, false, false,
                   false, false,
                   false, false, false);

XboxManager::XboxManager()
{
}

void XboxManager::begin()
{
  // begin Joystick
  Joystick.begin();

  //Set joystick x,y range
  Joystick.setXAxisRange(AXIS_UP_LEFT, AXIS_DOWN_RIGHT);
  Joystick.setYAxisRange(AXIS_UP_LEFT, AXIS_DOWN_RIGHT);
}

void XboxManager::buttonDownUp(int b)
{
  Joystick.setButton(b, true);
  delay(BUTTON_DOWN_UP_DELAY);
  Joystick.setButton(b, false);
}

void XboxManager::buttonDown(int b)
{
  Joystick.setButton(b, true);
}

void XboxManager::buttonUp(int b)
{
  Joystick.setButton(b, false);
}

void XboxManager::setButton(int b, boolean v)
{
  Joystick.setButton(b, v);
}

void XboxManager::setXAxis(int x)
{
  Joystick.setXAxis(x);
}

void XboxManager::setYAxis(int y)
{
  Joystick.setYAxis(y);
}

void XboxManager::reset()
{
  for (int i = 0; i < 7; i++)
  {
    Joystick.releaseButton(i);
  }
  setXAxis(AXIS_MIDDLE);
  setYAxis(AXIS_MIDDLE);
}
