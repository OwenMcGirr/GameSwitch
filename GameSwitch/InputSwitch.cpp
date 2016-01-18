/*
 * A class to represent an input switch.
 */

#include "InputSwitch.h"

InputSwitch::InputSwitch(int p) {
  // set pin
  pin = p;

  // set pin mode
  pinMode(pin, INPUT);

  // set defaults
  currentState = LOW;
  previousState = LOW;
}

void InputSwitch::debounce() {
  boolean current = digitalRead(pin);

  if (current != previousState) {
    delay(DEBOUNCE_DELAY);
    current = digitalRead(pin);
  }

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
