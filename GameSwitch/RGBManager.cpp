/*
 * Basic class for controlling a RGB LED.
 */

#include "RGBManager.h"

RGBManager::RGBManager(int r, int g, int b) {
  // set pins
  redPin = r;
  greenPin = g;
  bluePin = b;

  // set pin modes
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void RGBManager::setColor(int r, int g, int b) {
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}

void RGBManager::doCycle() {
  setColor(255, 0, 0);
  delay(CYCLE_DELAY);
  setColor(0, 255, 0);
  delay(CYCLE_DELAY);
  setColor(0, 0, 255);
  delay(CYCLE_DELAY);
  setColor(0, 0, 0);
  delay(CYCLE_DELAY);
}
