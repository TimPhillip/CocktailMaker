#ifndef I2CSCAN_FRAME_H
#define I2CSCAN_FRAME_H

#include <display.h>
#include <Wire.h>

class I2CScanFrame : public Frame {

public:

  I2CScanFrame(Adafruit_ILI9341*);

  void begin();
  void update();
};

#endif
