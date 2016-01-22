#define DEBOUNCE_DELAY 10

// switch pins
int inputSwitchA = 3;
int inputSwitchB = 4;
int inputSwitchC = 5;

// led pin
int led = 8;

// pin state variables
boolean currentInputSwitchAState = LOW;
boolean previousInputSwitchAState = LOW;
boolean currentInputSwitchBState = LOW;
boolean previousInputSwitchBState = LOW;
boolean currentInputSwitchCState = LOW;
boolean previousInputSwitchCState = LOW;

void setup() {
  // initialise IO
  pinMode(inputSwitchA, INPUT);
  pinMode(inputSwitchB, INPUT);
  pinMode(inputSwitchC, INPUT);
  pinMode(led, OUTPUT);
}

void loop() {
  // debounce switches
  debounceSwitches();

  // if a switch is pressed, light led
  if (currentInputSwitchAState == HIGH || currentInputSwitchBState == HIGH || currentInputSwitchCState == HIGH) {
    digitalWrite(led, HIGH);
  }
  else {
    digitalWrite(led, LOW);
  }

  // set previous input switch states
  setPreviousSwitchStates();
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
