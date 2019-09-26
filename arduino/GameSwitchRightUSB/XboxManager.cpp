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
  Joystick.pressButton(b);
  delay(DOWN_UP_DELAY);
  Joystick.releaseButton(b);
}

void XboxManager::buttonDown(int b) {
  Joystick.pressButton(b);
}

void XboxManager::buttonUp(int b) {
  Joystick.releaseButton(b);
}

void XboxManager::reset() {
  for (int i = 0; i < 7; i++) {
    Joystick.releaseButton(i);
  }
}
