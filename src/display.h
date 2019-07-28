#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

class Frame {

public:

  Frame(Adafruit_ILI9341*);
  virtual void begin();
  virtual void update();
  virtual void onTouchEvent(int touch_x, int touch_y);

protected:

  Adafruit_ILI9341* display;

};




class DisplayManager{

public:

  DisplayManager();
  void setFrame(Frame*);
  Frame* getFrame();

  void update();
  void onTouchEvent(TS_Point tp);

private:

  Frame* currentFrame;


};



#endif
