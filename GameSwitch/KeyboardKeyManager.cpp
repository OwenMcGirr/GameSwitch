/*
 * Basic class for controlling keyboard keys.
 */

#include "KeyboardKeyManager.h"

KeyboardKeyManager::KeyboardKeyManager() {
  // begin keyboard
  Keyboard.begin();
}

void KeyboardKeyManager::keyDownUp(char k) {
  Keyboard.press(k);
  delay(DOWN_UP_DELAY);
  Keyboard.release(k);
}

void KeyboardKeyManager::keyDown(char k) {
  Keyboard.press(k);
}

void KeyboardKeyManager::keyUp(char k) {
  Keyboard.release(k);
}

void KeyboardKeyManager::keyDownUp(int k) {
  Keyboard.press(k);
  delay(DOWN_UP_DELAY);
  Keyboard.release(k);
}

void KeyboardKeyManager::keyDown(int k) {
  Keyboard.press(k);
}

void KeyboardKeyManager::keyUp(int k) {
  Keyboard.release(k);
}

void KeyboardKeyManager::reset() {
  Keyboard.releaseAll();
}
