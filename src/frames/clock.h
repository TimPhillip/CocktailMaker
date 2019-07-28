#ifndef CLOCK_FRAME_H
#define CLOCK_FRAME_H

#include <display.h>
#include <NTPClient.h>

class ClockFrame : public Frame {

public:

  ClockFrame(NTPClient*, Adafruit_ILI9341*);

  void begin();
  void update();

private:

  NTPClient* ntpClient;

};

#endif
