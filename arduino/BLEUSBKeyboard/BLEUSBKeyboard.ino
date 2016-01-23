#include <SPI.h>
#include "Adafruit_BLE_UART.h"

// uart object parameters
#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 2
#define ADAFRUITBLE_RST 9

// uart object
Adafruit_BLE_UART uart(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

void setup(void) {
  // wait for serial
  while (!Serial);

  // start serial
  Serial.begin(9600);

  // start uart
  if (!uart.begin()) {
    Serial.println("Bluefruit not found!!!");
  }
  else {
    Serial.println("Bluefruit found!!! UART started!!!");
  }
}

void loop(void) {
}
