#include "timing.h"
#include "modes.h"
#include "walking_actions.h"

// input switch pins
int inputSwitchA = 3;
int inputSwitchB = 4;

// RGB pins
int redLED = 8;
int greenLED = 7;
int blueLED = 6;

// pin state variables
boolean currentInputSwitchAState = LOW;
boolean previousInputSwitchAState = LOW;
boolean currentInputSwitchBState = LOW;
boolean previousInputSwitchBState = LOW;

// input switch A press count variables
int inputSwitchAPressCount = 0;
boolean inputSwitchAPressCountActive = false;

// input switch A last press time variables
unsigned long inputSwitchALastPressTime = 0;
boolean inputSwitchALastPressTimeActive = false;

// input switch B hold variables
unsigned long startInputSwitchBTime = 0;
unsigned long pressedInputSwitchBTime = 0;
boolean freshInputSwitchBTime = true;

// mode variables
int currentMode = 1;

// walking mode variables
boolean sprinting = false;

void setup() {
  // initialise IO
  pinMode(inputSwitchA, INPUT);
  pinMode(inputSwitchB, INPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  // set walking mode
  setMode(WALKING_MODE);

  // start keyboard
  Keyboard.begin();
}

void loop() {
  // debounce switches
  debounceSwitches();

  // update input switch B hold time
  updateInputSwitchBHoldTime();

  // walking mode
  if (isWalkingMode()) {
    // if input switch A was just released, increment count and record time
    if (wasInputSwitchAJustReleased()) {
      incrementInputSwitchAPressCount();

      resetInputSwitchALastPressTime();
      recordInputSwitchALastPressTime();
    }

    // if input switch A hasn't been pressed for the duration of the take action timeout, stop current action and do selected action
    if (shouldTakeAction() && isInputSwitchAPressCountActive()) {
      // stop current action
      stopAction();

      // do selected action
      switch (inputSwitchAPressCount) {
        case FORWARD:
          walkForward();
          break;
        case BACKWARD:
          walkBackward();
          break;
        case LEFT:
          walkLeft();
          break;
        case RIGHT:
          walkRight();
          break;
        case JUMP:
          jump();
          break;
        case SPRINT:
          toggleSprint();
          break;
        case ENTER_OR_EXIT:
          enterOrExit();
          break;
        default:
          stopAction();
          break;
      }

      resetInputSwitchAPressCount();
      resetInputSwitchALastPressTime();
    }

    // if switch B was just released and , stop current action
    if (wasInputSwitchBJustReleased() && pressedInputSwitchBTime < SWITCH_HOLD_1) {
      stopAction();
    }
  }

  // fighting mode
  if (isFightingMode()) {
    if (wasInputSwitchAJustReleased()) {
      fire();
    }
  }

  // if switch B is held for the duration of the third hold time, go to next mode
  if (isInputSwitchBPressed() && pressedInputSwitchBTime == SWITCH_HOLD_3) {
    nextMode();
  }

  // check if input switch B hold time should be reset
  checkShouldResetInputSwitchBHoldTime();

  // set previous input switch states
  setPreviousSwitchStates();
}


/*
 * Switch state return functions
 */

boolean isInputSwitchAPressed() {
  return currentInputSwitchAState == HIGH;
}

boolean isInputSwitchBPressed() {
  return currentInputSwitchBState == HIGH;
}

boolean wasInputSwitchAJustReleased() {
  return currentInputSwitchAState == LOW && previousInputSwitchAState == HIGH;
}

boolean wasInputSwitchBJustReleased() {
  return currentInputSwitchBState == LOW && previousInputSwitchBState == HIGH;
}


/*
 * Mode logic functions
 */

void setMode(int mode) {
  resetModes();

  currentMode = mode;

  cycleRGB();

  setModeIndication();
}

void nextMode() {
  switch (currentMode) {
    case WALKING_MODE:
      setMode(DRIVING_MODE);
      break;
    case DRIVING_MODE:
      setMode(FIGHTING_MODE);
      break;
    case FIGHTING_MODE:
      setMode(WALKING_MODE);
      break;
  }
}

void setModeIndication() {
  switch (currentMode) {
    case WALKING_MODE:
      setRGBColor(255, 0, 0);
      break;
    case DRIVING_MODE:
      setRGBColor(0, 255, 0);
      break;
    case FIGHTING_MODE:
      setRGBColor(0, 0, 255);
      break;
  }
}

boolean isWalkingMode() {
  return currentMode == WALKING_MODE;
}

boolean isDrivingMode() {
  return currentMode == DRIVING_MODE;
}

boolean isFightingMode() {
  return currentMode == FIGHTING_MODE;
}


/*
 * Common mode functions
 */

void stopAction() {
  Keyboard.release('w');
  Keyboard.release('s');
  Keyboard.release('a');
  Keyboard.release('d');
}

void resetModes() {
  resetWalkingMode();

  stopAction();
}


/*
 * Walking mode functions
 */

void walkForward() {
  Keyboard.press('w');
}

void walkBackward() {
  Keyboard.press('s');
}

void walkLeft() {
  Keyboard.press('a');
}

void walkRight() {
  Keyboard.press('d');
}

void jump() {
  Keyboard.write('j');
}

void toggleSprint() {
  if (!sprinting) {
    Keyboard.press(' ');
    sprinting = true;
  }
  else {
    Keyboard.release(' ');
    sprinting = false;
  }
}

void enterOrExit() {
  Keyboard.write('\n');
}

void resetWalkingMode() {
  Keyboard.release(' ');
  sprinting = false;
}


/*
 * Driving mode functions
 */

void accelerate() {
  Keyboard.press('w');
}

void reverse() {
  Keyboard.press('s');
}

void steerLeft() {
  Keyboard.write('a');
}

void steerRight() {
  Keyboard.write('d');
}


/*
 * Fighting mode functions
 */

void fire() {
  Keyboard.write('o');
}


/*
 * Timing and counting functions
 */

void incrementInputSwitchAPressCount() {
  inputSwitchAPressCount++;
  inputSwitchAPressCountActive = true;
}

void resetInputSwitchAPressCount() {
  inputSwitchAPressCount = 0;
  inputSwitchAPressCountActive = false;
}

boolean isInputSwitchAPressCountActive() {
  return inputSwitchAPressCountActive;
}

void recordInputSwitchALastPressTime() {
  inputSwitchALastPressTime = millis();
  inputSwitchALastPressTimeActive = true;
}

void resetInputSwitchALastPressTime() {
  inputSwitchALastPressTime = 0;
  inputSwitchALastPressTimeActive = false;
}

boolean shouldTakeAction() {
  return inputSwitchALastPressTime < (millis() - TAKE_ACTION_TIMEOUT) && inputSwitchALastPressTimeActive;
}

void updateInputSwitchBHoldTime() {
  // if input switch B is pressed
  if (currentInputSwitchBState == HIGH) {
    if (freshInputSwitchBTime) { // input switch B has just been pressed, start timing hold
      startInputSwitchBTime = millis();
      freshInputSwitchBTime = false;
    }
    else { // update hold time
      pressedInputSwitchBTime = millis() - startInputSwitchBTime;
    }
  }
}

void checkShouldResetInputSwitchBHoldTime() {
  // if input switch B is released, reset hold time
  if (currentInputSwitchBState == LOW) {
    startInputSwitchBTime = 0;
    pressedInputSwitchBTime = 0;
    freshInputSwitchBTime = true;
  }
}


/*
 * RGB functions
 */

void setRGBColor(unsigned int red, unsigned int green, unsigned int blue) {
  analogWrite(redLED, red);
  analogWrite(greenLED, green);
  analogWrite(blueLED, blue);
}

void cycleRGB() {
  setRGBColor(255, 0, 0);
  delay(RGB_CYCLE_DELAY);
  setRGBColor(0, 255, 0);
  delay(RGB_CYCLE_DELAY);
  setRGBColor(0, 0, 255);
  delay(RGB_CYCLE_DELAY);
  setRGBColor(0, 0, 0);
  delay(RGB_CYCLE_DELAY);
}


/*
 * Debounce functions
 */

boolean debounce(int pin, boolean previous) {
  boolean current = digitalRead(pin);

  if (current != previous) {
    delay(DEBOUNCE_DELAY);
    current = digitalRead(pin);
  }

  return current;
}

void debounceSwitches() {
  currentInputSwitchAState = debounce(inputSwitchA, previousInputSwitchAState);
  currentInputSwitchBState = debounce(inputSwitchB, previousInputSwitchBState);
}

void setPreviousSwitchStates() {
  previousInputSwitchAState = currentInputSwitchAState;
  previousInputSwitchBState = currentInputSwitchBState;
}

