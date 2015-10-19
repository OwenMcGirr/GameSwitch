#define DEBOUNCE_DELAY 10
#define RGB_CYCLE_DELAY 500

// switch pins
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

void setup() {
  // initialise IO
  pinMode(inputSwitchA, INPUT);
  pinMode(inputSwitchB, INPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  // cycle RGB
  cycleRGB();
}

void loop() {
  // debounce switches
  debounceSwitches();

  // set previous input switch states
  setPreviousSwitchStates();
}


/*
 * RGB functions
 */

void setColorRGB(unsigned int red, unsigned int green, unsigned int blue) {
  analogWrite(redLED, red);
  analogWrite(greenLED, green);
  analogWrite(blueLED, blue);
}

void cycleRGB() {
  setColorRGB(255, 0, 0);
  delay(RGB_CYCLE_DELAY);
  setColorRGB(0, 255, 0);
  delay(RGB_CYCLE_DELAY);
  setColorRGB(0, 0, 255);
  delay(RGB_CYCLE_DELAY);
  setColorRGB(0, 0, 0);
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

