#include "timing.h"
#include "modes.h"
#include "InputSwitch.h"
#include "RGBManager.h"

// input switches
InputSwitch inputSwitchA(3);
InputSwitch inputSwitchB(4);
InputSwitch inputSwitchC(5);

// RGB manager
RGBManager rgbManager(8, 7, 6);

// input switch A press count and last press time variables
int inputSwitchAPressCount = 0; // incremented every time input switch A is pressed
boolean inputSwitchAPressCountActive = false; // count only takes place when this is true
unsigned long inputSwitchALastPressTime = 0; // recorded every time input switch A is pressed
boolean inputSwitchALastPressTimeActive = false; // record only when this is true
boolean shouldDoExtraFunctions = false; // when true, input switch A can do different actions based on the number of times the user presses it

// mode logic variables
int currentMode; // which mode the device is currently in

// walking and driving mode variables
boolean walkingForwardOrAccelerating = false; // whether or not you are walking forward or accelerating
boolean walkingBackwardOrReversing = false; // whether or not you are walking backward or reversing
boolean sprinting = false; // whether or not you are sprinting
boolean brakeOnTurn = false; // whether or not you brake on turn while driving

// menu mode variables
char menuStyle = 'h'; // whether the menu is horizontal or vertical, 'h' or 'v'

void setup() {
  // set walking mode
  setMode(WALKING_AND_DRIVING_MODE);

  // start keyboard and mouse
  Keyboard.begin();
  Mouse.begin();

  // start serial
  Serial.begin(9600);
}

void loop() {
  // debounce switches
  debounceSwitches();




  // update input switch B hold time
  inputSwitchB.updateHoldTime();




  // if walking and driving or menu mode, check for extra function activation
  if (isWalkingAndDrivingMode() || isMenuMode()) {
    // if switch C was just released, enable extra functions
    if (inputSwitchC.wasJustReleased()) {
      shouldDoExtraFunctions = true;

      prepareToDoAnExtraFunction();
    }

    // if switch A was just released, increment count and record time
    if (inputSwitchA.wasJustReleased() && shouldDoExtraFunctions) {
      incrementInputSwitchAPressCount();
      recordInputSwitchALastPressTime();
    }
  }




  // walking and driving mode
  if (isWalkingAndDrivingMode()) {
    // if switch A was just released and not walking, accelerating or reversing, walk or accelerate
    if (inputSwitchA.wasJustReleased() && !walkingForwardOrAccelerating && !walkingBackwardOrReversing && !shouldDoExtraFunctions) {
      toggleWalkOrAccelerate();
    }

    // if walking, accelerating or reversing, switch A and B act as left and right
    if (walkingForwardOrAccelerating || walkingBackwardOrReversing) {
      if (inputSwitchA.isDown()) {
        prepareForTurn();
        walkOrSteerLeftDown();
        Serial.println("walk or steer left down");
      }
      else if (inputSwitchB.isDown()) {
        prepareForTurn();
        walkOrSteerRightDown();
        Serial.println("walk or steer right down");
      }
      else if (inputSwitchA.wasJustReleased() || inputSwitchB.wasJustReleased()) {
        releaseWSADKeys();
        chooseDirectionAfterTurn();
        Serial.println("walking, accelerating or reversing");
      }
    }

    // check should do extra function
    checkShouldDoExtraWalkingAndDrivingModeFunction();
  }




  // menu mode
  if (isMenuMode()) {
    if (!shouldDoExtraFunctions) {
      // horizontal menu
      if (menuStyle == 'h') {
        if (inputSwitchA.wasJustReleased()) {
          doMenuLeft();
        }
        if (inputSwitchB.wasJustReleased()) {
          doMenuRight();
        }
      }
      // vertical menu
      else if (menuStyle == 'v') {
        if (inputSwitchA.wasJustReleased()) {
          doMenuUp();
        }
        if (inputSwitchB.wasJustReleased()) {
          doMenuDown();
        }
      }
    }

    // check should do extra function
    checkShouldDoExtraMenuModeFunction();
  }




  // fighting mode
  if (isFightingMode()) {
    if (inputSwitchA.wasJustReleased()) {
      fire();
    }
    if (inputSwitchB.wasJustReleased()) {
      nextWeapon();
    }
    if (inputSwitchC.wasJustReleased()) {
      reloadWeapon();
    }
  }




  // if switch B is held for the duration of the third hold time and not walking, accelerating or reversing, go to next mode
  if (inputSwitchB.getHoldTime() == SWITCH_HOLD_3 && !walkingForwardOrAccelerating && !walkingBackwardOrReversing) {
    nextMode();
  }




  // check if input switch B hold time should be reset
  inputSwitchB.checkShouldResetHoldTime();




  // set previous input switch states
  setPreviousSwitchStates();
}


/*
 * Mode logic functions
 */

void setMode(int mode) {
  resetModes();

  currentMode = mode;

  rgbManager.doCycle();

  setModeIndication();
}

void nextMode() {
  switch (currentMode) {
    case WALKING_AND_DRIVING_MODE:
      setMode(FIGHTING_MODE);
      break;
    case FIGHTING_MODE:
      setMode(MENU_MODE);
      break;
    case MENU_MODE:
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
      rgbManager.setColor(255, 0, 0);
      break;
    case FIGHTING_MODE:
      rgbManager.setColor(0, 255, 0);
      break;
    case MENU_MODE:
      rgbManager.setColor(0, 0, 255);
      break;
    case REST_MODE:
      rgbManager.setColor(0, 0, 0);
      break;
  }
}

boolean isWalkingAndDrivingMode() {
  return currentMode == WALKING_AND_DRIVING_MODE;
}

boolean isFightingMode() {
  return currentMode == FIGHTING_MODE;
}

boolean isMenuMode() {
  return currentMode == MENU_MODE;
}

boolean isRestMode() {
  return currentMode == REST_MODE;
}


/*
 * Common mode functions
 */

void resetModes() {
  Keyboard.releaseAll();

  walkingForwardOrAccelerating = false;
  walkingBackwardOrReversing = false;
  sprinting = false;

  shouldDoExtraFunctions = false;
}

void prepareToDoAnExtraFunction() {
  releaseWSADKeys();

  walkingForwardOrAccelerating = false;
  walkingBackwardOrReversing = false;
}


/*
 * Walking and driving mode functions
 */

void toggleWalkOrAccelerate() {
  if (walkingBackwardOrReversing) {
    toggleReverse();
  }

  if (!walkingForwardOrAccelerating) {
    Keyboard.press('w');
    walkingForwardOrAccelerating = true;
  }
  else {
    Keyboard.release('w');
    walkingForwardOrAccelerating = false;
  }
}

void toggleReverse() {
  if (walkingForwardOrAccelerating) {
    toggleWalkOrAccelerate();
  }

  if (!walkingBackwardOrReversing) {
    Keyboard.press('s');
    walkingBackwardOrReversing = true;
  }
  else {
    Keyboard.release('s');
    walkingBackwardOrReversing = false;
  }
}

void chooseDirectionAfterTurn() {
  if (walkingForwardOrAccelerating) {
    Keyboard.press('w');
  }
  else if (walkingBackwardOrReversing) {
    Keyboard.press('s');
  }
}

void toggleBrakeOnTurn() {
  if (!brakeOnTurn) {
    brakeOnTurn = true;
  }
  else {
    brakeOnTurn = false;
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

void prepareForTurn() {
  if (!brakeOnTurn) {
    Keyboard.release('w');
    Keyboard.release('s');
  }
  else {
    Keyboard.press('s');
  }
}

void walkOrSteerLeftDown() {
  Keyboard.press('a');
}

void walkOrSteerRightDown() {
  Keyboard.press('d');
}

void releaseWSADKeys() {
  Keyboard.release('w');
  Keyboard.release('s');
  Keyboard.release('a');
  Keyboard.release('d');
}

void checkShouldDoExtraWalkingAndDrivingModeFunction() {
  // switch A, press a certain amount of times for different actions, these can (usually) be assigned to any game action
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
        toggleBrakeOnTurn();
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
    shouldDoExtraFunctions = false;
  }
}


/*
 * Fighting mode functions
 */

void fire() {
  mouseDownUp(MOUSE_LEFT, KEY_PULSE_DELAY);
}

void nextWeapon() {
  mouseDownUp(MOUSE_RIGHT, KEY_PULSE_DELAY);
}

void reloadWeapon() {
  keyDownUp(KEY_F5, KEY_PULSE_DELAY);
}


/*
 * Menu mode functions
 */

void doMenuSelect() {
  keyDownUp(KEY_RETURN, KEY_PULSE_DELAY);
}

void doMenuBack() {
  keyDownUp(KEY_ESC, KEY_PULSE_DELAY);
}

void doMenuUp() {
  keyDownUp(KEY_UP_ARROW, KEY_PULSE_DELAY);
}

void doMenuDown() {
  keyDownUp(KEY_DOWN_ARROW, KEY_PULSE_DELAY);
}

void doMenuLeft() {
  keyDownUp(KEY_LEFT_ARROW, KEY_PULSE_DELAY);
}

void doMenuRight() {
  keyDownUp(KEY_RIGHT_ARROW, KEY_PULSE_DELAY);
}

// horizontal or vertical menu
void switchMenuStyle() {
  if (menuStyle == 'h') {
    menuStyle = 'v';
  }
  else {
    menuStyle = 'h';
  }
}

void checkShouldDoExtraMenuModeFunction() {
  // switch A, press a certain amount of times for different actions
  if (shouldTakeInputSwitchAPressCountAction()) {
    // do selected action
    switch (inputSwitchAPressCount) {
      case 1:
        doMenuSelect();
        break;
      case 2:
        doMenuBack();
        break;
      case 3:
        switchMenuStyle();
        break;
    }

    resetInputSwitchAPressCount();
    resetInputSwitchALastPressTime();
    shouldDoExtraFunctions = false;
  }
}


/*
 * Key and mouse press functions
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

void mouseDownUp(int button, unsigned long delayMillis) {
  Mouse.press(button);
  delay(delayMillis);
  Mouse.release(button);
}


/*
 * Timing and counting functions
 */

// used to count the number of times input switch A is pressed
void incrementInputSwitchAPressCount() {
  inputSwitchAPressCount++;
  inputSwitchAPressCountActive = true;
}

void resetInputSwitchAPressCount() {
  inputSwitchAPressCount = 0;
  inputSwitchAPressCountActive = false;
}

// record the last time input switch A was pressed
void recordInputSwitchALastPressTime() {
  inputSwitchALastPressTime = millis();
  inputSwitchALastPressTimeActive = true;
}

void resetInputSwitchALastPressTime() {
  inputSwitchALastPressTime = 0;
  inputSwitchALastPressTimeActive = false;
}

// returns whether or not the extra function should take place
boolean shouldTakeInputSwitchAPressCountAction() {
  return inputSwitchALastPressTime < (millis() - SWITCH_A_TAKE_ACTION_TIMEOUT) && inputSwitchALastPressTimeActive;
}


/*
 * Debounce functions
 */

void debounceSwitches() {
  inputSwitchA.debounce();
  inputSwitchB.debounce();
  inputSwitchC.debounce();
}

void setPreviousSwitchStates() {
  inputSwitchA.setPreviousState();
  inputSwitchB.setPreviousState();
  inputSwitchC.setPreviousState();
}

