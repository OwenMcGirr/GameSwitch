/*
 * A class to represent an input switch.
 */

#include "InputSwitch.h"

InputSwitch::InputSwitch() {
  // set defaults
  currentState = LOW;
  previousState = LOW;
}

void InputSwitch::setCurrentState(boolean current) {
  currentState = current;
}

void InputSwitch::setPreviousState() {
  previousState = currentState;
}

void InputSwitch::updateHoldTime() {
  if (currentState == HIGH) {
    holdTimer.updateTimer();
  }
}

void InputSwitch::checkShouldResetHoldTime() {
  if (currentState == LOW) {
    holdTimer.resetTimer();
  }
}

long InputSwitch::getHoldTime() {
  return holdTimer.getElapsedTime();
}

boolean InputSwitch::isDown() {
  return currentState == HIGH;
}

boolean InputSwitch::wasJustReleased() {
  return currentState == LOW && previousState == HIGH;
}
