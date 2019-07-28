#ifndef SETUP_FRAME_H
#define SETUP_FRAME_H

#include <display.h>
#include <frames/button.h>
#include <recept.pb.h>

class SetupFrame: public Frame {

  public:

    SetupFrame(DisplayManager* display_manager, Adafruit_ILI9341* display);

    void begin();
    void update();
    void onTouchEvent(int touch_x, int touch_y);

  private:
    Button* home_button;
    DisplayManager* display_manager;

};


#endif
