/*
   Basic class for controlling Xbox.
*/

#include "XboxManager.h"

XboxManager::XboxManager() {
}

void XboxManager::begin() {
  // begin Joystick
  Joystick.begin();

  //Set joystick x,y range
  Joystick.setXAxisRange(AXIS_UP_LEFT, AXIS_DOWN_RIGHT);
  Joystick.setYAxisRange(AXIS_UP_LEFT, AXIS_DOWN_RIGHT);
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

void XboxManager::setXAxis(int x) {
  Joystick.setXAxis(x);
}

void XboxManager::setYAxis(int y) {
  Joystick.setYAxis(y);
}

void XboxManager::reset() {
}
