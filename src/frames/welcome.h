#ifndef WELCOME_FRAME_H
#define WELCOME_FRAME_H

#include <display.h>
#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <frames/button.h>
#include <rendering.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>


class WelcomeFrame: public Frame{

public:
    WelcomeFrame(Adafruit_ILI9341*);
    void begin();
    void update();
    void onTouchEvent(int x, int y);

private:
    Button* button1;
    Button* button2;
    Button* button3;
    Button* button4;

    int hours;
    int minutes;
    int connection;

};


#endif
