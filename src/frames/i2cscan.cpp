#include <frames/i2cscan.h>

I2CScanFrame::I2CScanFrame(Adafruit_ILI9341* display) : Frame(display){
}

void I2CScanFrame::begin(){
  Frame::begin();
  this->display->setTextSize(2);
  this->display->println("I2C Devices:");
  this->display->println("");
}


void I2CScanFrame::update(){


  // Check all the addreses
  for (int i =1; i< 127; i++){

    Wire.beginTransmission(i);
    int error = Wire.endTransmission();

    if (error == 0){
      this->display->print("- 0x");
      this->display->print(i, HEX);
      this->display->println("");
    }
  }
}
