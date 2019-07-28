#ifndef BUTTON_H
#define BUTTON_H

#include<Arduino.h>
#include<Adafruit_ILI9341.h>

class Button{

  public:
    Button(int x, int y, int width, int height, Adafruit_ILI9341* display);

    void setText(String text);
    void setColor(int color);

    void draw();
    bool isTouched(int touch_x, int touch_y);

  private:
    Adafruit_ILI9341* display;
    int x;
    int y;
    int width;
    int height;
    String text;
    int color;

};


#endif
