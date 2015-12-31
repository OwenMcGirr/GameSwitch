#include "timing.h"
#include "modes.h"

// input switch pins
int inputSwitchA = 3;
int inputSwitchB = 4;
int inputSwitchC = 5;

// RGB pins
int redLED = 8;
int greenLED = 7;
int blueLED = 6;

// pin state variables
boolean currentInputSwitchAState = LOW;
boolean previousInputSwitchAState = LOW;
boolean currentInputSwitchBState = LOW;
boolean previousInputSwitchBState = LOW;
boolean currentInputSwitchCState = LOW;
boolean previousInputSwitchCState = LOW;

// input switch A press count and last press time variables
int inputSwitchAPressCount = 0;
boolean inputSwitchAPressCountActive = false;
unsigned long inputSwitchALastPressTime = 0;
boolean inputSwitchALastPressTimeActive = false;
boolean shouldDoFKeys = false;

// input switch B hold variables
unsigned long startInputSwitchBTime = 0;
unsigned long pressedInputSwitchBTime = 0;
boolean freshInputSwitchBTime = true;

// mode variables
int currentMode = 1;

// walking and driving mode variables
boolean walkingOrAccelerating = false;
boolean reversing = false;
boolean sprinting = false;

void setup() {
  // initialise IO
  pinMode(inputSwitchA, INPUT);
  pinMode(inputSwitchB, INPUT);
  pinMode(inputSwitchC, INPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  // set walking mode
  setMode(WALKING_AND_DRIVING_MODE);

  // start keyboard
  Keyboard.begin();

  // start serial
  Serial.begin(9600);
}

void loop() {
  // debounce switches
  debounceSwitches();

  // update input switch B hold time
  updateInputSwitchBHoldTime();

  // walking and driving mode
  if (isWalkingAndDrivingMode()) {
    // if switch A was just released and not walking, accelerating or reversing, walk or accelerate
    if (wasInputSwitchAJustReleased() && !walkingOrAccelerating && !reversing && !shouldDoFKeys) {
      toggleWalkOrAccelerate();
    }

    // if walking, accelerating or reversing, switch A and B act as left and right
    if (walkingOrAccelerating || reversing) {
      if (isInputSwitchAPressed()) {
        decelerateOrStopWalkingForTurn();
        walkOrSteerLeftDown();
        Serial.println("walk or steer left down");
      }
      else if (isInputSwitchBPressed()) {
        decelerateOrStopWalkingForTurn();
        walkOrSteerRightDown();
        Serial.println("walk or steer right down");
      }
      else if (wasInputSwitchAJustReleased() || wasInputSwitchBJustReleased()) {
        resetWalkingAndDrivingMode();
        toggleWalkOrAccelerate();
        Serial.println("walk or drive forward");
      }
    }
  }

  // if switch C was just released, enable F keys
  if (wasInputSwitchCJustReleased()) {
    resetWalkingAndDrivingMode();
    shouldDoFKeys = true;
  }

  // if switch A was just released, increment count and record time
  if (wasInputSwitchAJustReleased() && shouldDoFKeys) {
    incrementInputSwitchAPressCount();
    recordInputSwitchALastPressTime();
  }

  // switch A, press a certain amount of times for different actions
  if (shouldTakeInputSwitchAPressCountAction()) {
    // do selected action
    switch (inputSwitchAPressCount) {
      case 1:
        keyDownUp(KEY_F1, KEY_PULSE_DELAY);
        break;
      case 2:
        keyDownUp(KEY_F2, KEY_PULSE_DELAY);
        break;
      case 3:
        toggleReverse();
        break;
      case 4:
        pauseOrResume();
        break;
      case 5:
        keyDownUp(KEY_F3, KEY_PULSE_DELAY);
        break;
      case 6:
        keyDownUp(KEY_F4, KEY_PULSE_DELAY);
        break;
      case 7:
        toggleSprint();
        break;
      case 8:
        keyDownUp(KEY_F7, KEY_PULSE_DELAY);
        break;
      case 9:
        keyDownUp(KEY_F8, KEY_PULSE_DELAY);
        break;
      case 10:
        keyDownUp(KEY_F9, KEY_PULSE_DELAY);
        break;
      case 11:
        keyDownUp(KEY_F10, KEY_PULSE_DELAY);
        break;
      case 12:
        keyDownUp(KEY_F11, KEY_PULSE_DELAY);
        break;
      case 13:
        keyDownUp(KEY_F12, KEY_PULSE_DELAY);
        break;
    }

    resetInputSwitchAPressCount();
    resetInputSwitchALastPressTime();
    shouldDoFKeys = false;
  }

  // fighting mode
  if (isFightingMode()) {
    if (wasInputSwitchAJustReleased() && !shouldDoFKeys) {
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

boolean isInputSwitchCPressed() {
  return currentInputSwitchCState == HIGH;
}

boolean wasInputSwitchAJustReleased() {
  return currentInputSwitchAState == LOW && previousInputSwitchAState == HIGH;
}

boolean wasInputSwitchBJustReleased() {
  return currentInputSwitchBState == LOW && previousInputSwitchBState == HIGH;
}

boolean wasInputSwitchCJustReleased() {
  return currentInputSwitchCState == LOW && previousInputSwitchCState == HIGH;
}


/*
 * Mode logic functions
 */

void setMode(int mode) {
  resetWalkingAndDrivingMode();

  currentMode = mode;

  cycleRGB();

  setModeIndication();
}

void nextMode() {
  switch (currentMode) {
    case WALKING_AND_DRIVING_MODE:
      setMode(FIGHTING_MODE);
      break;
    case FIGHTING_MODE:
      setMode(REST_MODE);
      break;
    case REST_MODE:
      setMode(WALKING_AND_DRIVING_MODE);
      break;
  }
}

void setModeIndication() {
  switch (currentMode) {
    case WALKING_AND_DRIVING_MODE:
      setRGBColor(255, 0, 0);
      break;
    case FIGHTING_MODE:
      setRGBColor(0, 0, 255);
      break;
    case REST_MODE:
      setRGBColor(0, 0, 0);
      break;
  }
}

boolean isWalkingAndDrivingMode() {
  return currentMode == WALKING_AND_DRIVING_MODE;
}

boolean isFightingMode() {
  return currentMode == FIGHTING_MODE;
}

boolean isRestMode() {
  return currentMode == REST_MODE;
}


/*
 * Common mode functions
 */

void pauseOrResume() {
  keyDownUp(KEY_ESC, KEY_PULSE_DELAY);
}


/*
 * Walking and driving mode functions
 */

void toggleWalkOrAccelerate() {
  if (reversing) {
    toggleReverse();
  }

  if (!walkingOrAccelerating) {
    Keyboard.press('w');
    walkingOrAccelerating = true;
  }
  else {
    Keyboard.release('w');
    walkingOrAccelerating = false;
  }
}

void toggleReverse() {
  if (walkingOrAccelerating) {
    toggleWalkOrAccelerate();
  }

  if (!reversing) {
    Keyboard.press('s');
    reversing = true;
  }
  else {
    Keyboard.release('s');
    reversing = false;
  }
}

void toggleSprint() {
  if (!sprinting) {
    Keyboard.press(KEY_F6);
    sprinting = true;
  }
  else {
    Keyboard.release(KEY_F6);
    sprinting = false;
  }
}

void decelerateOrStopWalkingForTurn() {
  Keyboard.release('w');
  Keyboard.release('s');
}

void walkOrSteerLeftDown() {
  Keyboard.press('a');
}

void walkOrSteerRightDown() {
  Keyboard.press('d');
}

void resetWalkingAndDrivingMode() {
  Keyboard.releaseAll();

  walkingOrAccelerating = false;
  reversing = false;
  sprinting = false;
}


/*
 * Fighting mode functions
 */

void fire() {
  keyDownUp(KEY_F5, KEY_PULSE_DELAY);
}


/*
 * Key functions
 */

void keyDownUp(char key, unsigned long delayMillis) {
  Keyboard.press(key);
  delay(delayMillis);
  Keyboard.release(key);
}

void keyDownUp(int key, unsigned long delayMillis) {
  Keyboard.press(key);
  delay(delayMillis);
  Keyboard.release(key);
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

void recordInputSwitchALastPressTime() {
  inputSwitchALastPressTime = millis();
  inputSwitchALastPressTimeActive = true;
}

void resetInputSwitchALastPressTime() {
  inputSwitchALastPressTime = 0;
  inputSwitchALastPressTimeActive = false;
}

boolean shouldTakeInputSwitchAPressCountAction() {
  return inputSwitchALastPressTime < (millis() - SWITCH_A_TAKE_ACTION_TIMEOUT) && inputSwitchALastPressTimeActive;
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
  currentInputSwitchCState = debounce(inputSwitchC, previousInputSwitchCState);
}

void setPreviousSwitchStates() {
  previousInputSwitchAState = currentInputSwitchAState;
  previousInputSwitchBState = currentInputSwitchBState;
  previousInputSwitchCState = currentInputSwitchCState;
}
