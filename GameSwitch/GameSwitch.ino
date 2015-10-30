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

// input switch B hold variables
unsigned long startInputSwitchBTime = 0;
unsigned long pressedInputSwitchBTime = 0;
boolean freshInputSwitchBTime = true;

// input switch C press count variables
int inputSwitchCPressCount = 0;
boolean inputSwitchCPressCountActive = false;

// input switch C last press time variables
unsigned long inputSwitchCLastPressTime = 0;
boolean inputSwitchCLastPressTimeActive = false;

// mode variables
int currentMode = 1;

// walking and driving mode variables
boolean walkingOrAccelerating = false;
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
    if (wasInputSwitchAJustReleased() && !walkingOrAccelerating && !reversing) {
      toggleWalkOrAccelerate();
    }

    // if walking, accelerating or reversing, switch A and B act as left and right
    if (walkingOrAccelerating || reversing) {
      if (isInputSwitchAPressed()) {
        walkOrSteerLeftDown();
        Serial.println("walk or steer left down");
      }
      else if (isInputSwitchBPressed()) {
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
      setMode(CHEAT_MODE);
      break;
    case CHEAT_MODE:
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
    case CHEAT_MODE:
      setRGBColor(255, 255, 255);
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

boolean isCheatMode() {
  return currentMode == CHEAT_MODE;
}

boolean isRestMode() {
  return currentMode == REST_MODE;
}


/*
 * Common mode functions
 */

void enterOrExit() {
  keyDownUp('\n', KEY_PULSE_DELAY);
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

void walkOrSteerLeftDown() {
  Keyboard.press('a');
}

void walkOrSteerRightDown() {
  Keyboard.press('d');
}

void jump() {
  keyDownUp('j', KEY_PULSE_DELAY);
}

void resetWalkingAndDrivingMode() {
  Keyboard.releaseAll();

  walkingOrAccelerating = false;
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

void incrementInputSwitchCPressCount() {
  inputSwitchCPressCount++;
  inputSwitchCPressCountActive = true;
}

void resetInputSwitchCPressCount() {
  inputSwitchCPressCount = 0;
  inputSwitchCPressCountActive = false;
}

void recordInputSwitchCLastPressTime() {
  inputSwitchCLastPressTime = millis();
  inputSwitchCLastPressTimeActive = true;
}

void resetInputSwitchCLastPressTime() {
  inputSwitchCLastPressTime = 0;
  inputSwitchCLastPressTimeActive = false;
}

boolean shouldTakeAction() {
  return inputSwitchCLastPressTime < (millis() - TAKE_ACTION_TIMEOUT) && inputSwitchCLastPressTimeActive;
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
