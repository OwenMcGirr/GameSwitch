#include "timing.h"
#include "modes.h"
#include "walking_actions.h"

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

// driving mode variables
boolean accelerating = false;
boolean reversing = false;

void setup() {
  // initialise IO
  pinMode(inputSwitchA, INPUT);
  pinMode(inputSwitchB, INPUT);
  pinMode(inputSwitchC, INPUT);
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

    // if switch B was just released and was held for less time than the duration of hold 1, stop current action
    if (wasInputSwitchBJustReleased() && pressedInputSwitchBTime < SWITCH_HOLD_1) {
      stopAction();
    }
  }

  // driving mode
  if (isDrivingMode()) {
    // if switch A was just released and not accelerating or reversing, accelerate
    if (wasInputSwitchAJustReleased() && !accelerating && !reversing) {
      toggleAccelerate();
    }

    // if accelerating or reversing, switch A and B act as left and right
    if (accelerating || reversing) {
      if (isInputSwitchAPressed()) {
        steerLeftDown();
      }
      else {
        steerLeftUp();
      }

      if (isInputSwitchBPressed() && pressedInputSwitchBTime < SWITCH_HOLD_2) {
        steerRightDown();
      }
      else {
        steerRightUp();
      }
    }

    // if switch B is held for the duration of hold 2, reverse
    if (isInputSwitchBPressed() && pressedInputSwitchBTime == SWITCH_HOLD_2) {
      toggleReverse();
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
      setMode(CHEAT_MODE);
      break;
    case CHEAT_MODE:
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
    case CHEAT_MODE:
      setRGBColor(255, 255, 255);
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

boolean isCheatMode() {
  return currentMode == CHEAT_MODE;
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
  resetDrivingMode();

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
  keyDownUp('a', WALK_LEFT_RIGHT_DELAY);
}

void walkRight() {
  keyDownUp('d', WALK_LEFT_RIGHT_DELAY);
}

void jump() {
  keyDownUp('j', KEY_PULSE_DELAY);
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
  keyDownUp('\n', KEY_PULSE_DELAY);
}

void resetWalkingMode() {
  Keyboard.release(' ');
  sprinting = false;
}


/*
 * Driving mode functions
 */

void toggleAccelerate() {
  if (reversing) {
    toggleReverse();
  }
  
  if (!accelerating) {
    Keyboard.press('w');
    accelerating = true;
  }
  else {
    Keyboard.release('w');
    accelerating = false;
  }
}

void toggleReverse() {
  if (accelerating) {
    toggleAccelerate();
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

void steerLeftDown() {
  Keyboard.press('a');
}

void steerRightDown() {
  Keyboard.press('d');
}

void steerLeftUp() {
  Keyboard.release('a');
}

void steerRightUp() {
  Keyboard.release('d');
}

void resetDrivingMode() {
  accelerating = false;
  reversing = false;
}


/*
 * Fighting mode functions
 */

void fire() {
  keyDownUp('o', KEY_PULSE_DELAY);
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
  currentInputSwitchCState = debounce(inputSwitchC, previousInputSwitchCState);
}

void setPreviousSwitchStates() {
  previousInputSwitchAState = currentInputSwitchAState;
  previousInputSwitchBState = currentInputSwitchBState;
  previousInputSwitchCState = currentInputSwitchCState;
}
