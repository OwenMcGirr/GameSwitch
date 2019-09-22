#ifndef RGBManager_h
#define RGBManager_h

#include "Arduino.h"

#define CYCLE_DELAY 250

class RGBManager
{
  public:
    RGBManager(int r, int g, int b);
    void setColor(int r, int g, int b);
    void doCycle();
  private:
    int redPin;
    int bluePin;
    int greenPin;
};

#endif
