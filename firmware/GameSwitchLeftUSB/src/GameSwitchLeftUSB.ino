#include <SPI.h>
#if not defined(_VARIANT_ARDUINO_DUE_X_) && not defined(_VARIANT_ARDUINO_ZERO_)
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
int inputSwitchAPressCount = 0;                  // incremented every time input switch A is pressed
boolean inputSwitchAPressCountActive = false;    // count only takes place when this is true
unsigned long inputSwitchALastPressTime = 0;     // recorded every time input switch A is pressed
boolean inputSwitchALastPressTimeActive = false; // record only when this is true
boolean shouldDoExtraFunctions = false;          // when true, input switch A can do different actions based on the number of times the user presses it

// mode logic variables
int currentMode; // which mode the device is currently in
boolean shouldChangeMode = false;

// walking and driving mode variables
boolean walkingForwardOrAccelerating = false; // whether or not you are walking forward or accelerating
boolean walkingBackwardOrReversing = false;   // whether or not you are walking backward or reversing
char directionForwardOrBackward = 'f';        // what direction you are walking or driving in
boolean sprinting = false;
boolean turnWithCamera = false;
boolean decelerateOnTurn = true;
boolean brakeOnTurn = false;
boolean shouldHandbrake = false;
char handbrakeButton = 'a';
boolean autoFire = false; // whether or not auto fire is on
boolean autoFireInProgress = false;
Timer autoFireInterruptTimer;

// fighting mode variables
boolean aiming = false;

// football mode variables
char directionLeftOrRight = 'l';
boolean movingFootballPlayer = false;

// menu mode variables
char menuStyle = 'h'; // whether the menu is horizontal or vertical, 'h' or 'v'

// bluetooth variables
boolean btWasDisconnected = false;

void setup()
{
  // start serial
  Serial.begin(9600);

  // start xbox
  xboxManager.begin();

  // start ble
  if (!ble.begin(true))
  {
    Serial.println("Bluefruit not found!!!");
  }
  else
  {
    Serial.println("Bluefruit found!!! BLE started!!!");
  }

  // set broadcast name
  if (!ble.sendCommandCheckOK(F("AT+GAPDEVNAME=GameSwitchLeftUSB")))
  {
    Serial.println("Could not set broadcast name!!!");
  }

  // print info
  ble.info();

  // disable verbose
  ble.verbose(false);

  // set mode
  ble.setMode(BLUEFRUIT_MODE_DATA);

  // wait for connection
  while (!ble.isConnected())
  {
    delay(15000);
  }

  // set walking mode
  setMode(WALKING_MODE);
}

void loop()
{
  while (!ble.isConnected())
  {
    btWasDisconnected = true;
    xboxManager.reset();
    delay(15000);
  }
  if (btWasDisconnected)
  {
    broadcastModeIndication();
    btWasDisconnected = false;
  }

  // update switches
  updateSwitches();

  // update input switch B hold time
  inputSwitchB.updateHoldTime();
  //Serial.println(inputSwitchB.getHoldTime());

  if (!shouldChangeMode)
  {
    // if walking, driving, football, or menu mode, check for extra function activation
    if (isWalkingMode() || isDrivingMode() || isFootballMode() || isMenuMode())
    {
      // if switch C was just released, enable or disable extra functions
      if (inputSwitchC.wasJustReleased())
      {
        if (!shouldDoExtraFunctions)
        {
          shouldDoExtraFunctions = true;
          prepareToDoAnExtraFunction();
        }
        else
        {
          resetInputSwitchAPressCount();
          resetInputSwitchALastPressTime();
          shouldDoExtraFunctions = false;
          resetModes();
        }
      }

      // if switch A was just released, increment count and record time
      if (inputSwitchA.wasJustReleased() && shouldDoExtraFunctions)
      {
        incrementInputSwitchAPressCount();
        recordInputSwitchALastPressTime();
      }
    }

    // walking and driving mode
    if (isWalkingMode() || isDrivingMode())
    {
      // if switch A was just released and not walking, accelerating or reversing, walk or accelerate
      if (inputSwitchA.wasJustReleased() && directionForwardOrBackward == 'n' && !shouldDoExtraFunctions)
      {
        toggleWalkOrAccelerate();
      }

      // if walking, accelerating or reversing, switch A and B act as left and right
      if (walkingForwardOrAccelerating || walkingBackwardOrReversing)
      {
        if ((inputSwitchA.wasJustPressed() || inputSwitchB.wasJustPressed()) && isDrivingMode())
        {
          prepareForTurn();
        }
        if (inputSwitchA.wasJustPressed())
        {
          if (isWalkingMode() && turnWithCamera)
          {
            xboxManager.setYAxis(AXIS_MIDDLE);
            ble.print(RIGHT_STICK_LEFT);
          }
        }
        else if (inputSwitchB.wasJustPressed())
        {
          if (isWalkingMode() && turnWithCamera)
          {
            xboxManager.setYAxis(AXIS_MIDDLE);
            ble.print(RIGHT_STICK_RIGHT);
          }
        }
        if (inputSwitchA.isDown())
        {
          if ((isWalkingMode() && !turnWithCamera) || isDrivingMode())
          {
            walkOrSteerLeftDown();
          }

          Serial.println("walk or steer left down");
        }
        else if (inputSwitchB.isDown())
        {
          if ((isWalkingMode() && !turnWithCamera) || isDrivingMode())
          {
            walkOrSteerRightDown();
          }

          Serial.println("walk or steer right down");
        }
        else if (inputSwitchA.wasJustReleased() || inputSwitchB.wasJustReleased())
        {
          xboxManager.setXAxis(AXIS_MIDDLE);
          chooseDirectionAfterTurn();
          Serial.println("walking, accelerating or reversing");
        }
      }

      // auto fire
      if (autoFireInProgress)
      {
        autoFireWeapon();
      }
    }

    if (isWalkingMode() || isDrivingMode() || isFootballMode())
    {
      // check should do extra function
      checkShouldDoExtraWalkingDrivingOrFootballModeFunction();
    }

    // football mode
    if (isFootballMode())
    {
      if (!shouldDoExtraFunctions)
      {
        movingFootballPlayer = true;
        if (inputSwitchA.isDown())
        {
          if (directionLeftOrRight == 'l')
          {
            xboxManager.setYAxis(AXIS_DOWN_RIGHT);
          }
          else
          {
            xboxManager.setYAxis(AXIS_UP_LEFT);
          }
        }
        else if (inputSwitchB.isDown())
        {
          if (directionLeftOrRight == 'l')
          {
            xboxManager.setYAxis(AXIS_UP_LEFT);
          }
          else
          {
            xboxManager.setYAxis(AXIS_DOWN_RIGHT);
          }
        }
        else if (inputSwitchA.wasJustReleased() || inputSwitchB.wasJustReleased())
        {
          xboxManager.setYAxis(AXIS_MIDDLE);
          if (directionLeftOrRight == 'l')
          {
            xboxManager.setXAxis(AXIS_UP_LEFT);
          }
          else
          {
            xboxManager.setXAxis(AXIS_DOWN_RIGHT);
          }
        }
      }
      else
      {
        xboxManager.setButton(B_BUTTON, inputSwitchB.isDown());
      }
    }

    // menu mode
    if (isMenuMode())
    {
      if (!shouldDoExtraFunctions)
      {
        // horizontal menu
        if (menuStyle == 'h')
        {
          if (inputSwitchA.wasJustReleased())
          {
            doMenuLeft();
          }
          if (inputSwitchB.wasJustReleased())
          {
            doMenuRight();
          }
        }
        // vertical menu
        else if (menuStyle == 'v')
        {
          if (inputSwitchA.wasJustReleased())
          {
            doMenuUp();
          }
          if (inputSwitchB.wasJustReleased())
          {
            doMenuDown();
          }
        }
      }

      // check should do extra function
      checkShouldDoExtraMenuModeFunction();
    }

    // fighting mode
    if (isFightingMode())
    {
      if (inputSwitchA.wasJustReleased())
      {
        fire();
      }
      if (inputSwitchB.wasJustReleased())
      {
        toggleAim();
      }
      if (inputSwitchC.wasJustReleased())
      {
        reloadWeapon();
      }
    }

    if (inputSwitchB.getHoldTime() >= SWITCH_HOLD_TIME && !walkingForwardOrAccelerating && !walkingBackwardOrReversing && !movingFootballPlayer)
    {
      shouldChangeMode = true;
    }
  }
  else
  {
    if (inputSwitchA.wasJustReleased())
    {
      nextMode();
    }
    if (inputSwitchB.wasJustReleased() && inputSwitchB.getHoldTime() < SWITCH_HOLD_TIME)
    {
      shouldChangeMode = false;
      broadcastModeIndication();
    }
  }

  // check if input switch B hold time should be reset
  inputSwitchB.checkShouldResetHoldTime();

  // set previous input switch states
  setPreviousSwitchStates();
}

/*
   Mode logic functions
*/

void setMode(int mode)
{
  resetModes();

  currentMode = mode;

  delay(150);
  broadcastModeIndication();

  Serial.print("mode=");
  Serial.println(mode);
}

void nextMode()
{
  switch (currentMode)
  {
  case WALKING_MODE:
    setMode(DRIVING_MODE);
    break;
  case DRIVING_MODE:
    setMode(FIGHTING_MODE);
    break;
  case FIGHTING_MODE:
    setMode(FOOTBALL_MODE);
    break;
  case FOOTBALL_MODE:
    setMode(MENU_MODE);
    break;
  case MENU_MODE:
    setMode(WALKING_MODE);
    break;
  }
}

void broadcastModeIndication()
{
  switch (currentMode)
  {
  case WALKING_MODE:
    ble.print("Walking Mode");
    break;
  case DRIVING_MODE:
    ble.print("Driving Mode");
    break;
  case FIGHTING_MODE:
    ble.print("Fighting Mode");
    break;
  case FOOTBALL_MODE:
    ble.print("Football Mode");
    break;
  case MENU_MODE:
    ble.print("Menu Mode");
    break;
  }
}

boolean isWalkingMode()
{
  return currentMode == WALKING_MODE;
}

boolean isDrivingMode()
{
  return currentMode == DRIVING_MODE;
}

boolean isFightingMode()
{
  return currentMode == FIGHTING_MODE;
}

boolean isFootballMode()
{
  return currentMode == FOOTBALL_MODE;
}

boolean isMenuMode()
{
  return currentMode == MENU_MODE;
}

/*
   Common mode functions
*/

void resetModes()
{
  resetXbox();

  walkingForwardOrAccelerating = false;
  walkingBackwardOrReversing = false;
  directionForwardOrBackward = 'n';

  aiming = false;

  autoFireInProgress = false;

  movingFootballPlayer = false;

  shouldDoExtraFunctions = false;
}

void prepareToDoAnExtraFunction()
{
  resetXbox();

  walkingForwardOrAccelerating = false;
  walkingBackwardOrReversing = false;

  movingFootballPlayer = false;
}

void toggleDirectionChange()
{
  if (isFootballMode())
  {
    if (directionLeftOrRight == 'l')
    {
      directionLeftOrRight = 'r';
      xboxManager.setXAxis(AXIS_DOWN_RIGHT);
    }
    else
    {
      directionLeftOrRight = 'l';
      xboxManager.setXAxis(AXIS_UP_LEFT);
    }
    return;
  }

  if (directionForwardOrBackward == 'f')
  {
    toggleWalkOrAccelerate();
  }

  if (directionForwardOrBackward != 'b')
  {
    if (isDrivingMode())
    {
      xboxManager.buttonDown(LEFT_TRIGGER_BUTTON);
    }
    else
    {
      xboxManager.setYAxis(AXIS_DOWN_RIGHT);
      xboxManager.setButton(A_BUTTON, sprinting);
    }
    walkingBackwardOrReversing = true;
    directionForwardOrBackward = 'b';
  }
  else
  {
    resetXbox();
    walkingBackwardOrReversing = false;
    directionForwardOrBackward = 'n';
    delay(500);
    toggleWalkOrAccelerate();
  }
}

void checkShouldDoExtraWalkingDrivingOrFootballModeFunction()
{
  // switch A, press a certain amount of times for different actions, these can (usually) be assigned to any game action
  if (shouldTakeInputSwitchAPressCountAction())
  {
    // do selected action
    switch (inputSwitchAPressCount)
    {
    case 1:
      toggleDirectionChange();
      break;
    case 2:
      xboxManager.buttonDownUp(A_BUTTON);
      break;
    case 3:
      xboxManager.buttonDownUp(B_BUTTON);
      break;
    case 4:
      ble.print(TAP_X);
      break;
    case 5:
      ble.print(TAP_Y);
      break;
    case 6:
      toggleSprint();
      break;
    case 7:
      toggleAutoFire();
      break;
    case 8:
      toggleBrakeOnTurn();
      break;
    case 9:
      toggleDecelerateOnTurn();
      break;
    case 10:
      toggleTurnWithCamera();
      break;
    case 11:
      xboxManager.buttonDownUp(LEFT_BUMPER_BUTTON);
      break;
    case 12:
      switchHandbrakeButton();
      break;
    }

    if (inputSwitchAPressCount > 1)
    {
      directionForwardOrBackward = 'n';
      movingFootballPlayer = false;
    }

    resetInputSwitchAPressCount();
    resetInputSwitchALastPressTime();
    shouldDoExtraFunctions = false;

    if ((isWalkingMode() || isDrivingMode()) && inputSwitchAPressCount > 1)
    {
      delay(500);
      toggleWalkOrAccelerate();
    }
  }
}

/*
   Walking and driving mode functions
*/

void toggleWalkOrAccelerate()
{
  if (directionForwardOrBackward != 'f')
  {
    if (isDrivingMode())
    {
      ble.print(TOGGLE_ACCELERATE);
    }
    else
    {
      xboxManager.setYAxis(AXIS_UP_LEFT);
      xboxManager.setButton(A_BUTTON, sprinting);
    }
    walkingForwardOrAccelerating = true;
    directionForwardOrBackward = 'f';
  }
  else
  {
    resetXbox();
    walkingForwardOrAccelerating = false;
  }
}

void chooseDirectionAfterTurn()
{
  if (directionForwardOrBackward == 'f' || directionForwardOrBackward == 'n')
  {
    if (isDrivingMode())
    {
      if (!brakeOnTurn && decelerateOnTurn)
      {
        ble.print(TOGGLE_ACCELERATE);
      }
      else if (brakeOnTurn)
      {
        xboxManager.setButton(LEFT_TRIGGER_BUTTON, false);
      }
      xboxManager.buttonUp(A_BUTTON);
      xboxManager.buttonUp(B_BUTTON);
    }
    else
    {
      ble.print(RIGHT_STICK_MIDDLE);
      xboxManager.setYAxis(AXIS_UP_LEFT);
    }
  }
  else
  {
    if (isDrivingMode())
    {
      xboxManager.buttonDown(LEFT_TRIGGER_BUTTON);
      xboxManager.buttonUp(A_BUTTON);
      xboxManager.buttonUp(B_BUTTON);
    }
    else
    {
      ble.print(RIGHT_STICK_MIDDLE);
      xboxManager.setYAxis(AXIS_DOWN_RIGHT);
    }
  }
}

void toggleSprint()
{
  sprinting = !sprinting;
  xboxManager.setButton(A_BUTTON, sprinting);
}

void toggleTurnWithCamera()
{
  turnWithCamera = !turnWithCamera;
}

void toggleDecelerateOnTurn()
{
  decelerateOnTurn = !decelerateOnTurn;
  if (decelerateOnTurn)
  {
    brakeOnTurn = false;
  }
}

void toggleBrakeOnTurn()
{
  brakeOnTurn = !brakeOnTurn;
  if (brakeOnTurn)
  {
    decelerateOnTurn = false;
  }
}

void toggleHandbrake()
{
  shouldHandbrake = !shouldHandbrake;
}

void switchHandbrakeButton()
{
  switch (handbrakeButton)
  {
  case 'a':
    handbrakeButton = 'b';
    break;
  case 'b':
    handbrakeButton = 'a';
    break;
  default:
    break;
  }
}

void pressHandbrakeButton()
{
  if (shouldHandbrake)
  {
    switch (handbrakeButton)
    {
    case 'a':
      xboxManager.buttonDown(A_BUTTON);
      break;
    case 'b':
      xboxManager.buttonDown(B_BUTTON);
      break;
    default:
      break;
    }
    shouldHandbrake = false;
  }
}

void toggleAutoFire()
{
  autoFire = !autoFire;
}

void prepareForTurn()
{
  if (brakeOnTurn)
  {
    xboxManager.setButton(LEFT_TRIGGER_BUTTON, true);
  }
  else if (decelerateOnTurn)
  {
    resetXbox();
  }
}

void walkOrSteerLeftDown()
{
  xboxManager.setXAxis(AXIS_UP_LEFT);

  if (isDrivingMode())
  {
    pressHandbrakeButton();
  }
}

void walkOrSteerRightDown()
{
  xboxManager.setXAxis(AXIS_DOWN_RIGHT);

  if (isDrivingMode())
  {
    pressHandbrakeButton();
  }
}

/*
   Fighting mode functions
*/

void fire()
{
  ble.print(FIRE);
}

void autoFireWeapon()
{
  if (autoFireInterruptTimer.getElapsedTime() > 1200)
  {
    resetXbox();

    walkingForwardOrAccelerating = false;
    walkingBackwardOrReversing = false;
    directionForwardOrBackward = 'n';

    delay(100);

    if (!aiming)
    {
      toggleAim();
    }

    delay(100);

    fire();

    delay(100);

    toggleAim();

    autoFireInterruptTimer.resetTimer();
  }
  else
  {
    autoFireInterruptTimer.updateTimer();
  }
}

void toggleAim()
{
  aiming = !aiming;
  xboxManager.setButton(LEFT_TRIGGER_BUTTON, aiming);
}

void reloadWeapon()
{
  xboxManager.buttonDownUp(B_BUTTON);
}

void switchWeapon()
{
  xboxManager.buttonDown(LEFT_BUMPER_BUTTON);
  ble.print(WEAPON_CHANGE);
  delay(100);
  xboxManager.buttonUp(LEFT_BUMPER_BUTTON);
}

/*
   Menu mode functions
*/

void doMenuSelect()
{
  xboxManager.buttonDownUp(A_BUTTON);
}

void doMenuBack()
{
  xboxManager.buttonDownUp(B_BUTTON);
}

void doMenu()
{
  xboxManager.buttonDownUp(MENU_BUTTON);
}

void doMenuUp()
{
  xboxManager.setYAxis(AXIS_UP_LEFT);
  delay(DOWN_UP_DELAY);
  xboxManager.setYAxis(AXIS_MIDDLE);
}

void doMenuDown()
{
  xboxManager.setYAxis(AXIS_DOWN_RIGHT);
  delay(DOWN_UP_DELAY);
  xboxManager.setYAxis(AXIS_MIDDLE);
}

void doMenuLeft()
{
  xboxManager.setXAxis(AXIS_UP_LEFT);
  delay(DOWN_UP_DELAY);
  xboxManager.setXAxis(AXIS_MIDDLE);
}

void doMenuRight()
{
  xboxManager.setXAxis(AXIS_DOWN_RIGHT);
  delay(DOWN_UP_DELAY);
  xboxManager.setXAxis(AXIS_MIDDLE);
}

// horizontal or vertical menu
void switchMenuStyle()
{
  if (menuStyle == 'h')
  {
    menuStyle = 'v';
  }
  else
  {
    menuStyle = 'h';
  }
}

void escapeTestMode()
{
  xboxManager.setButton(MENU_BUTTON, true);
  xboxManager.setButton(VIEW_BUTTON, true);
  delay(DOWN_UP_DELAY);
  xboxManager.setButton(MENU_BUTTON, false);
  xboxManager.setButton(VIEW_BUTTON, false);
}

void checkShouldDoExtraMenuModeFunction()
{
  // switch A, press a certain amount of times for different actions
  if (shouldTakeInputSwitchAPressCountAction())
  {
    // do selected action
    switch (inputSwitchAPressCount)
    {
    case 1:
      doMenuSelect();
      break;
    case 2:
      doMenuBack();
      break;
    case 3:
      doMenu();
      break;
    case 4:
      switchMenuStyle();
      break;
    case 5:
      xboxManager.buttonDownUp(X1_BUTTON);
      break;
    case 6:
      xboxManager.buttonDownUp(LEFT_BUMPER_BUTTON);
      break;
    case 7:
      ble.print(TAP_RB);
      break;
    case 8:
      ble.print(TAP_X);
      break;
    case 9:
      ble.print(TAP_Y);
      break;
    case 10:
      xboxManager.buttonDownUp(VIEW_BUTTON);
      break;
    }

    resetInputSwitchAPressCount();
    resetInputSwitchALastPressTime();
    shouldDoExtraFunctions = false;
  }
}

/*
   Eye command 
*/

void doEyeCommand()
{
  if (isWalkingMode() && autoFire)
  {
    autoFireInProgress = !autoFireInProgress;
  }

  if (isDrivingMode())
  {
    toggleHandbrake();
  }

  if (isFightingMode())
  {
    switchWeapon();
  }

  if (isMenuMode())
  {
    switchMenuStyle();
  }
}

/*
   Reset
*/

void resetXbox()
{
  xboxManager.reset();
  ble.print(RESET);
}

/*
   Timing and counting functions
*/

// used to count the number of times input switch A is pressed
void incrementInputSwitchAPressCount()
{
  inputSwitchAPressCount++;
  inputSwitchAPressCountActive = true;
  Serial.print("inputSwitchAPressCount=");
  Serial.println(inputSwitchAPressCount);
}

void resetInputSwitchAPressCount()
{
  inputSwitchAPressCount = 0;
  inputSwitchAPressCountActive = false;
}

// record the last time input switch A was pressed
void recordInputSwitchALastPressTime()
{
  inputSwitchALastPressTime = millis();
  inputSwitchALastPressTimeActive = true;
}

void resetInputSwitchALastPressTime()
{
  inputSwitchALastPressTime = 0;
  inputSwitchALastPressTimeActive = false;
}

// returns whether or not the extra function should take place
boolean shouldTakeInputSwitchAPressCountAction()
{
  return inputSwitchALastPressTime < (millis() - SWITCH_A_TAKE_ACTION_TIMEOUT) && inputSwitchALastPressTimeActive;
}

/*
   Switch functions
*/

void updateSwitches()
{
  if (ble.available())
  {
    int c = ble.read();

    // print character
    Serial.println(c);

    switch (c)
    {
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
    case 55:
      doEyeCommand();
      break;
    }

    delay(10);
  }
}

void setPreviousSwitchStates()
{
  inputSwitchA.setPreviousState();
  inputSwitchB.setPreviousState();
  inputSwitchC.setPreviousState();
}
