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

XboxManager::XboxManager() {
}

void XboxManager::begin() {
  // begin Joystick
  Joystick.begin();
}

void XboxManager::buttonDownUp(int b) {
  Joystick.setButton(b, true);
  delay(DOWN_UP_DELAY);
  Joystick.setButton(b, false);
}

void XboxManager::buttonDown(int b) {
  Joystick.setButton(b, true);
}

void XboxManager::buttonUp(int b) {
  Joystick.setButton(b, false);
}

void XboxManager::reset() {
  for (int i = 0; i < 7; i++) {
    Joystick.releaseButton(i);
  }
}
