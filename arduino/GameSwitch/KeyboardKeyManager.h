#ifndef KeyboardKeyManager_h
#define KeyboardKeyManager_h

#include "Arduino.h"

#define DOWN_UP_DELAY 250

class KeyboardKeyManager
{
  public:
    KeyboardKeyManager();
    void keyDownUp(char k);
    void keyDown(char k);
    void keyUp(char k);
    void keyDownUp(int k);
    void keyDown(int k);
    void keyUp(int k);
    void reset();
  private:
    // none
};

#endif
