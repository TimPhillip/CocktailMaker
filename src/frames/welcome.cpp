#include "welcome.h"

extern Frame setup_frame;
extern DisplayManager display_manager;
extern NTPClient ntpClient;

WelcomeFrame::WelcomeFrame(Adafruit_ILI9341* display) :Frame(display){

  this->button1 = new Button(0,205,100,30, display);
  this->button1->setColor(ILI9341_WHITE);
  this->button1->setText("Setup");

  this->button2 = new Button(110,205,100,30, display);
  this->button2->setColor(ILI9341_WHITE);
  this->button2->setText("Cocktail");

  this->button3 = new Button(220,205,100,30, display);
  this->button3->setColor(ILI9341_WHITE);
  this->button3->setText("Clean");
}

void WelcomeFrame::begin(){

  // Clear the screen
  Frame::begin();

  this->display->setCursor(0,30);
  this->display->setTextSize(5);
  this->display->println(" Cocktail");
  this->display->println(" Maker");
  this->display->setTextSize(2);
  this->display->println();
  this->display->println("Familie Schneider");

  drawJpeg("/clean.jpg", 220, 80, display);

  // Buttons at the bottom
  this->button1->draw();
  this->button2->draw();
  this->button3->draw();

  // Reset setting
  this->connection = 0;
  this->hours = 0;
  this->minutes = 0;
}

void WelcomeFrame::update(){

  int status = WiFi.status();

  if ( status == WL_CONNECTED){

    if (status != this->connection){
      drawJpeg("/wifi.jpg", 290,0, display);
      this->connection = status;
    }

    ntpClient.update();
    int hours = ntpClient.getHours();
    int minutes = ntpClient.getMinutes();

    if (hours != this->hours || minutes != this->minutes){
      // Display the current time
      this->display->fillRect(0,0,100,30, ILI9341_BLACK);
      this->display->setCursor(0,0);
      if (hours < 10) this->display->print("0");
      this->display->print(hours);
      this->display->print(":");
      if (minutes < 10) this->display->print("0");
      this->display->print(minutes);

      this->hours = hours;
      this->minutes = minutes;
    }
  }

}

void WelcomeFrame::onTouchEvent(int touch_x, int touch_y){

  // Check if the Button is touched
  if (this->button1->isTouched(touch_x, touch_y)){
    display_manager.setFrame(&setup_frame);
  }

}
