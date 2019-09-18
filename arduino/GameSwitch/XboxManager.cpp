/*
   Basic class for controlling Xbox.
*/

#include "XboxManager.h"

XboxManager::XboxManager() {
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
}
