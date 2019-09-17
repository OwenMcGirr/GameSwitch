/*
   Basic class for controlling Xbox.
*/

#include "XboxManager.h"

XboxManager::XboxManager() {
  // begin Joystick
  Joystick.begin();
}

void XboxManager::keyDownUp(int k) {
  Joystick.pressButton(k);
  delay(DOWN_UP_DELAY);
  Joystick.releaseButton(k);
}

void XboxManager::keyDown(int k) {
  Joystick.pressButton(k);
}

void XboxManager::keyUp(int k) {
  Joystick.releaseButton(k);
}

void XboxManager::reset() {
}
