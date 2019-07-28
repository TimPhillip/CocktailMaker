#include<frames/button.h>


Button::Button(int x, int y, int width, int height, Adafruit_ILI9341* display){

    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->display = display;

    this->color = ILI9341_WHITE;
    this->text = "";
};

void Button::setText(String text){
  this->text = text;
}

void Button::setColor(int color){
  this->color = color;
}

void Button::draw(){
  this->display->drawRect(this->x, this->y, this->width, this->height, this->color);

  if ( this->text != ""){
    this->display->setTextColor(this->color);
    this->display->setCursor(this->x + 2, this->y + 7);
    this->display->print(this->text);
  }
}

bool Button::isTouched(int touch_x, int touch_y){

  if ( touch_x >= this->x && touch_y >= this->y ){

    if(touch_x <= this->x + this->width && touch_y <= this->y + this->height){
      return true;
    }

  }
  return false;
}
