#include <display.h>


Frame::Frame(Adafruit_ILI9341* display){

  // Get the reference to the display
  this->display = display;
}


void Frame::begin(){
  // Clear the TFT screen
  this->display->fillScreen(ILI9341_BLACK);
  this->display->setTextColor(ILI9341_WHITE);
  this->display->setCursor(0,0);
}

void Frame::update(){
  // Nothing to do here
  // Override this method in subclass
}

void Frame::onTouchEvent(int touch_x, int touch_y){

}


DisplayManager::DisplayManager(){

}

void DisplayManager::setFrame(Frame* next){
  // Change the frame
  next->begin();
  this->currentFrame = next;
}

Frame* DisplayManager::getFrame(){
  return this->currentFrame;
}

void DisplayManager::update(){
  // Delegate the update to the current frame
  this->currentFrame->update();
}

void DisplayManager::onTouchEvent(TS_Point tp){

  int x_min = 220;
  int x_max = 3669;
  int y_min = 280;
  int y_max = 3800;

  int x = 320 - (min(max(x_min, (int)tp.x), x_max) - x_min) * 320 / (x_max- x_min);
  int y = (min(max(y_min, (int)tp.y), y_max) - y_min) * 240 / (y_max- y_min);

  this->currentFrame->onTouchEvent(x,y);
}
