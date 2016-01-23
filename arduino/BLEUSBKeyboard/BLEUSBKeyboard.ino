#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluetoothConfig.h"

#include "KeyboardKeyManager.h"

// ble object
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// keyboardKeyManager object
KeyboardKeyManager keyboardKeyManager;

void setup(void) {
  // wait for serial
  while (!Serial);

  // start serial
  Serial.begin(9600);

  // start ble
  if (!ble.begin(true)) {
    Serial.println("Bluefruit not found!!!");
  }
  else {
    Serial.println("Bluefruit found!!! BLE started!!!");
  }

  // set broadcast name
  if (!ble.sendCommandCheckOK(F("AT+GAPDEVNAME=GameSwitchKeyboard"))) {
    Serial.println("Could not set broadcast name!!!");
  }

  // print info
  ble.info();

  // disable verbose
  ble.verbose(false);

  // wait for connection
  while (!ble.isConnected()) {
    delay(500);
  }

  // set mode
  ble.setMode(BLUEFRUIT_MODE_DATA);
}

void loop(void) {
  /// constantly poll for data
  while (ble.available()) {
    int c = ble.read();

    // print character
    Serial.println((char)c);

    // write character
    keyboardKeyManager.keyDownUp((char)c);

    delay(200);
  }
}
