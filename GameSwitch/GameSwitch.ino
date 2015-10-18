#define DEBOUNCE_DELAY 10

// switch pin
int inputSwitch = 3;

// RGB pins
int redLED = 8;
int greenLED = 7;
int blueLED = 6;

// pin state variables
boolean currentInputSwitchState = LOW;
boolean previousInputSwitchState = LOW;

void setup() {
  // initialise IO
  pinMode(inputSwitch, INPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
}

void loop() {
  // debounce switch
  currentInputSwitchState = debounce(inputSwitch, previousInputSwitchState);

  // set previous input switch state
  previousInputSwitchState = currentInputSwitchState;
}


/*
 * Debounce function
 */

boolean debounce(int pin, boolean previous) {
  boolean current = digitalRead(pin);

  if (current != previous) {
    delay(DEBOUNCE_DELAY);
    current = digitalRead(pin);
  }

  return current;
}
