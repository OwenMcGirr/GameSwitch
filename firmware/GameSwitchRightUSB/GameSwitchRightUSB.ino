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
  while (!ble.isConnected()) {
    xboxManager.reset();
    delay(500);
  }
  
  
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
  if (ble.available()) {
    char cmd = (char)ble.read();
    Serial.println(cmd);
    if (cmd == 'B') {
      toggleAccelerate();
    }
    else if (cmd == 'C') {
      xboxManager.buttonDownUp(RIGHT_TRIGGER_BUTTON);
    }
    else if (cmd == 'A') {
      xboxManager.reset();
      accelerating = false;
    }
    else if (cmd == 'D') {
      xboxManager.buttonDownUp(X_BUTTON);
    }
    else if (cmd == 'E') {
      xboxManager.buttonDownUp(Y_BUTTON);
    }
    delay(10);
  }
}
