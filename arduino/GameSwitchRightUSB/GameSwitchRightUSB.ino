#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BTConfig.h"

#include "XboxManager.h"

// ble object
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// Xbox manager
XboxManager xboxManager;

// variables
boolean accelerating = false;

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
  if (!ble.sendCommandCheckOK(F("AT+GAPDEVNAME=GameSwitchRightUSB"))) {
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
}

void loop() {
  readData();
}


void toggleAccelerate() {
  if (accelerating) {
    xboxManager.buttonUp(RIGHT_TRIGGER_BUTTON);
  }
  else {
    xboxManager.buttonDown(RIGHT_TRIGGER_BUTTON);
  }
  accelerating = !accelerating;
}


void readData() {
  char command[50];
  int len = 0;
  while (ble.available()) {
    command[len] = (char)ble.read();
    len++;
  }
  String cmd(command);
  if (cmd == "TOGGLE_ACCELERATE") {
    toggleAccelerate();
  }
  else if (cmd == "FIRE") {
    xboxManager.buttonDownUp(RIGHT_TRIGGER_BUTTON);
  }
  else if (cmd == "RESET") {
    xboxManager.reset();
    accelerating = false;
  }
}
