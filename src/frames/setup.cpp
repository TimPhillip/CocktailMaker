#include "setup.h"

extern Frame welcome_frame;
extern CocktailSetup cocktail_setup;

SetupFrame::SetupFrame( DisplayManager* display_manager, Adafruit_ILI9341* display) : Frame(display){
  this->home_button = new Button(0,210, 120, 25, display);
  this->home_button->setText("Home");
  this->home_button->setColor(ILI9341_WHITE);

  this->display_manager = display_manager;
}

void SetupFrame::begin() {
  Frame::begin();

  this->display->println("Current setup:");
  this->display->println();

  this->home_button->draw();
};


void SetupFrame::update(){

  this->display->setCursor(0,30);

  for (int i=0; i < cocktail_setup.liquids_count; i++){
    String out = "";
    out += "Pump ";
    out += String(cocktail_setup.liquids[i].pump_id);
    out += ": ";
    out += cocktail_setup.liquids[i].name;

    this->display->println(out);
  }
};

void SetupFrame::onTouchEvent(int touch_x, int touch_y){

  if (this->home_button->isTouched(touch_x, touch_y)){
    this->display_manager->setFrame(&welcome_frame);
  }

};
