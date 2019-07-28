#include <frames/clock.h>


ClockFrame::ClockFrame(NTPClient* ntpClient, Adafruit_ILI9341* display) : Frame(display){
  this->ntpClient = ntpClient;
}


void ClockFrame::begin(){

  Frame::begin();
  this->display->setTextSize(2);
  this->display->println("Current time:");
  this->display->println("");
}


void ClockFrame::update(){

  // Get the current time from server
  this->ntpClient->update();

  this->display->setTextSize(6);
  this->display->print(this->ntpClient->getHours());
  this->display->print(":");
  this->display->print(this->ntpClient->getMinutes());
}
