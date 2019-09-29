#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BTConfig.h"

#include "ble_commands.h"
#include "timing.h"
#include "modes.h"

#include "InputSwitch.h"
#include "XboxManager.h"

// ble object
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// input switches
InputSwitch inputSwitchA;
InputSwitch inputSwitchB;
InputSwitch inputSwitchC;

// Xbox manager
XboxManager xboxManager;

// input switch A press count and last press time variables
int inputSwitchAPressCount = 0; // incremented every time input switch A is pressed
boolean inputSwitchAPressCountActive = false; // count only takes place when this is true
unsigned long inputSwitchALastPressTime = 0; // recorded every time input switch A is pressed
boolean inputSwitchALastPressTimeActive = false; // record only when this is true
boolean shouldDoExtraFunctions = false; // when true, input switch A can do different actions based on the number of times the user presses it

// mode logic variables
int currentMode; // which mode the device is currently in
boolean didJustGoToNextMode; // will prevent irrational mode switches

// walking and driving mode variables
boolean walkingForwardOrAccelerating = false; // whether or not you are walking forward or accelerating
boolean walkingBackwardOrReversing = false; // whether or not you are walking backward or reversing

// menu mode variables
char menuStyle = 'h'; // whether the menu is horizontal or vertical, 'h' or 'v'

void setup() {
  while (!Serial);

  // start serial
  Serial.begin(9600);

  // start xbox
  xboxManager.begin();

  // start ble
  if (!ble.begin(true)) {
    Serial.println("Bluefruit not found!!!");
  }
  else {
    Serial.println("Bluefruit found!!! BLE started!!!");
  }

  // set broadcast name
  if (!ble.sendCommandCheckOK(F("AT+GAPDEVNAME=GameSwitchLeftUSB"))) {
    Serial.println("Could not set broadcast name!!!");
  }

  // print info
  ble.info();

  // disable verbose
  ble.verbose(false);

  // set mode
  ble.setMode(BLUEFRUIT_MODE_DATA);

  // wait for connection
  while (!ble.isConnected()) {
    delay(500);
  }

  // set walking mode
  setMode(WALKING_MODE);
}

void loop() {
  // update switches
  updateSwitches();




  // update input switch B hold time
  inputSwitchB.updateHoldTime();
  //Serial.println(inputSwitchB.getHoldTime());




  // if walking and driving or menu mode, check for extra function activation
  if (isWalkingMode() || isDrivingMode() || isMenuMode()) {
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
  if (isWalkingMode() || isDrivingMode()) {
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
        resetXbox();
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
  if (inputSwitchB.getHoldTime() >= SWITCH_HOLD_3 && !didJustGoToNextMode && !walkingForwardOrAccelerating && !walkingBackwardOrReversing) {
    nextMode();
    didJustGoToNextMode = true;
  }
  else if (inputSwitchB.wasJustReleased()) {
    didJustGoToNextMode = false;
  }




  // check if input switch B hold time should be reset
  inputSwitchB.checkShouldResetHoldTime();




  // set previous input switch states
  setPreviousSwitchStates();
}


/*
   Mode logic functions
*/

void setMode(int mode) {
  resetModes();

  currentMode = mode;

  broadcastModeIndication();

  Serial.print("mode=");
  Serial.println(mode);
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
      setMode(MENU_MODE);
      break;
    case MENU_MODE:
      setMode(REST_MODE);
      break;
    case REST_MODE:
      setMode(WALKING_MODE);
      break;
  }
}

void broadcastModeIndication() {
  switch (currentMode) {
    case WALKING_MODE:
      ble.println("Walking Mode");
      break;
    case DRIVING_MODE:
      ble.println("Driving Mode");
      break;
    case FIGHTING_MODE:
      ble.println("Fighting Mode");
      break;
    case MENU_MODE:
      ble.println("Menu Mode");
      break;
    case REST_MODE:
      ble.println("Rest Mode");
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

boolean isMenuMode() {
  return currentMode == MENU_MODE;
}

boolean isRestMode() {
  return currentMode == REST_MODE;
}


/*
   Common mode functions
*/

void resetModes() {
  resetXbox();

  walkingForwardOrAccelerating = false;
  walkingBackwardOrReversing = false;

  shouldDoExtraFunctions = false;
}

void prepareToDoAnExtraFunction() {
  resetXbox();

  walkingForwardOrAccelerating = false;
  walkingBackwardOrReversing = false;
}


/*
   Walking and driving mode functions
*/

void toggleWalkOrAccelerate() {
  if (walkingBackwardOrReversing) {
    toggleReverse();
  }

  if (!walkingForwardOrAccelerating) {
    if (isDrivingMode()) {
      ble.print(TOGGLE_ACCELERATE);
    }
    else {
      xboxManager.setXAxis(AXIS_UP_LEFT);
    }
    walkingForwardOrAccelerating = true;
  }
  else {
    resetXbox();
    walkingForwardOrAccelerating = false;
  }
}

void toggleReverse() {
  if (walkingForwardOrAccelerating) {
    toggleWalkOrAccelerate();
  }

  if (!walkingBackwardOrReversing) {
    if (isDrivingMode()) {
      xboxManager.buttonDown(LEFT_TRIGGER_BUTTON);
    }
    else {
      xboxManager.setXAxis(AXIS_DOWN_RIGHT);
    }
    walkingBackwardOrReversing = true;
  }
  else {
    resetXbox();
    walkingBackwardOrReversing = false;
  }
}

void chooseDirectionAfterTurn() {
  if (walkingForwardOrAccelerating) {
    if (isDrivingMode()) {
      ble.print(TOGGLE_ACCELERATE);
    }
    else {
      xboxManager.setXAxis(AXIS_UP_LEFT);
    }
  }
  else if (walkingBackwardOrReversing) {
    if (isDrivingMode()) {
      xboxManager.buttonDown(LEFT_TRIGGER_BUTTON);
    }
    else {
      xboxManager.setXAxis(AXIS_DOWN_RIGHT);
    }
  }
}

void prepareForTurn() {
  resetXbox();
}

void walkOrSteerLeftDown() {
  xboxManager.setYAxis(AXIS_UP_LEFT);
}

void walkOrSteerRightDown() {
  xboxManager.setYAxis(AXIS_DOWN_RIGHT);
}

void checkShouldDoExtraWalkingAndDrivingModeFunction() {
  // switch A, press a certain amount of times for different actions, these can (usually) be assigned to any game action
  if (shouldTakeInputSwitchAPressCountAction()) {
    // do selected action
    if (inputSwitchAPressCount == 1) {
      toggleReverse();
    }
    else {
      xboxManager.buttonDownUp(inputSwitchAPressCount - 2);
    }

    resetInputSwitchAPressCount();
    resetInputSwitchALastPressTime();
    shouldDoExtraFunctions = false;
  }
}


/*
   Fighting mode functions
*/

void fire() {
  xboxManager.buttonDownUp(X1_BUTTON);
}

void nextWeapon() {
}

void reloadWeapon() {
  xboxManager.buttonDownUp(B_BUTTON);
}


/*
   Menu mode functions
*/

void doMenuSelect() {
  xboxManager.buttonDownUp(X1_BUTTON);
}

void doMenuBack() {
  xboxManager.buttonDownUp(MENU_BUTTON);
}

void doMenuUp() {
  xboxManager.setXAxis(AXIS_UP_LEFT);
  delay(DOWN_UP_DELAY);
  xboxManager.setXAxis(AXIS_MIDDLE);
}

void doMenuDown() {
  xboxManager.setXAxis(AXIS_DOWN_RIGHT);
  delay(DOWN_UP_DELAY);
  xboxManager.setXAxis(AXIS_MIDDLE);
}

void doMenuLeft() {
  xboxManager.setYAxis(AXIS_UP_LEFT);
  delay(DOWN_UP_DELAY);
  xboxManager.setYAxis(AXIS_MIDDLE);
}

void doMenuRight() {
  xboxManager.setYAxis(AXIS_DOWN_RIGHT);
  delay(DOWN_UP_DELAY);
  xboxManager.setYAxis(AXIS_MIDDLE);
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
   Reset
*/

void resetXbox() {
  xboxManager.reset();
  ble.print(RESET);
}


/*
   Timing and counting functions
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
   Switch functions
*/

void updateSwitches() {
  if (ble.available()) {
    int c = ble.read();

    // print character
    Serial.println(c);

    switch (c) {
      case 49:
        inputSwitchA.setCurrentState(HIGH);
        break;
      case 50:
        inputSwitchA.setCurrentState(LOW);
        break;
      case 51:
        inputSwitchB.setCurrentState(HIGH);
        break;
      case 52:
        inputSwitchB.setCurrentState(LOW);
        break;
      case 53:
        inputSwitchC.setCurrentState(HIGH);
        break;
      case 54:
        inputSwitchC.setCurrentState(LOW);
        break;
    }

    delay(10);
  }
}

void setPreviousSwitchStates() {
  inputSwitchA.setPreviousState();
  inputSwitchB.setPreviousState();
  inputSwitchC.setPreviousState();
}
