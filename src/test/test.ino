#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "recept.pb.h"
#include <pb_decode.h>
#include <pb_encode.h>
#include <Ticker.h>
 
#include <Adafruit_ILI9341.h>
//#include <Adafruit_ImageReader.h>

// Individual pump speed values from calibration
float pump_speed[4] = {1.5,1.5,1.5, 1.35} ;//1.5245108352619399;
float pump_speed_correction[4] = {1.25/1.5, 1.35/1.5, 1.35/1.5, 1.5/1.5};

// Wifi setup
const char *ssid = "Hagelschlag";
const char *password = "12121212";

// REST server
ESP8266WebServer server;
const char* header_keys[1] = {"content-type"};

// The current cocktail machine setup
CocktailSetup cocktail_setup = CocktailSetup_init_zero;

void pump_off_callback(int);
long* calculate_durations(float* amount);
bool cocktail_in_make();

// Asynchronous task scheduling
Ticker pump_ticker[4];
Ticker progress_ticker;
int pump_pins[4] = {D1, D2, D3, D4};
float planned_amounts[4] = {0,0,0,0};
bool all_ingredients_found = true;
long planned_pump_durations[4] = {0,0,0,0};
long actual_pump_durations[4] = {0,0,0,0};

// Display setup
int CS_PIN = D0;
int DC_PIN = D8;
int SD_CS_PIN = D9;
Adafruit_ILI9341 tft = Adafruit_ILI9341(CS_PIN, DC_PIN);
//Adafruit_ImageReader img_reader;

void pump_async(int pump_id, unsigned long duration){
  digitalWrite(pump_pins[pump_id], LOW);
  pump_ticker[pump_id].once_ms(duration, pump_off_callback, pump_id);
}

void pump_off_callback(int pump_id){
  digitalWrite(pump_pins[pump_id], HIGH);
  pump_ticker[pump_id].detach();
}

void store_new_setup(){
  
  // Create an input stream from buffer
  unsigned int buffer_length = server.arg("proto").length();

  if (buffer_length == 0){
    server.send(400,"text", "Empty Buffer.");
  }

  unsigned char buffer[buffer_length];
  server.arg("proto").getBytes(buffer,buffer_length);
  pb_istream_t input_stream = pb_istream_from_buffer(buffer,buffer_length);

  // Decode the next Cocktail Setup
  if (!pb_decode(&input_stream, CocktailSetup_fields, &cocktail_setup)){
    server.send(400,"text", "Error while decoding setup.");
  }{
    server.send(200,"text",String(buffer_length));
  }
}

void display_current_setup(){

  if (server.header(1).startsWith("application/x-")){
    unsigned char buffer[1024];
    pb_ostream_t out = pb_ostream_from_buffer(buffer, 1024);
    pb_encode(&out, CocktailSetup_fields, &cocktail_setup);

    server.send_P(200, "test", (const char*)buffer, 1024);
  }else{
    String out = "Your current Cocktail Maker Setup:\n-------------------------------------\n";

    for (int i=0; i < cocktail_setup.liquids_count; i++){
      out += "Pump ";
      out += String(cocktail_setup.liquids[i].pump_id); 
      out += ":\t";
      out += cocktail_setup.liquids[i].name;
      out += "\n";
    }

    out += "Number of Liquids= " + String(cocktail_setup.liquids_count); 
    out += "\n";

    server.send(200, "text", out);
  }
}

void write_dummy_setup(){
  cocktail_setup.liquids[0].pump_id = 0;
  strcpy(cocktail_setup.liquids[0].name, "Cola");
  cocktail_setup.liquids_count++;

  cocktail_setup.liquids[1].pump_id = 1;
  strcpy(cocktail_setup.liquids[1].name, "Fanta");
  cocktail_setup.liquids_count++;

  cocktail_setup.liquids[2].pump_id = 2;
  strcpy(cocktail_setup.liquids[2].name, "Soda");
  cocktail_setup.liquids_count++;

  cocktail_setup.liquids[3].pump_id = 3;
  strcpy(cocktail_setup.liquids[3].name, "Rum");
  cocktail_setup.liquids_count++;
}

void calibrate(){
  int number_of_pumps = server.arg("pump").toInt();
  long duration = server.arg("duration").toInt();

  for (int i= 0; i < number_of_pumps; i++){
    pump_async(i, duration);
  }

  server.send(200, "text", "Calibrate for " + String(duration) + " milli-seconds.");
}

void update_progress(){

  if(! cocktail_in_make()){
    //memcpy(actual_pump_durations, planned_pump_durations,sizeof(planned_pump_durations[0]) * 4);
    progress_ticker.detach();
  }else{
    for(int i=0; i<4; i++){
      actual_pump_durations[i] = min(actual_pump_durations[i] + 500, planned_pump_durations[i]);
    }
  }

}

bool decode_ingredient(pb_istream_t *stream, const pb_field_t *field, void **arg){
  Ingredient ing = Ingredient_init_zero;
  if (pb_decode(stream,Ingredient_fields,&ing)){

    // Check if ingredient is in cocktail setup
    bool found = false;
    for(int i=0; i < cocktail_setup.liquids_count; i++){
      if(strcmp(ing.name, cocktail_setup.liquids[i].name) == 0){
        found = true;

        // add the amount to the plan
        planned_amounts[i] = ing.amount;
      }
    }

    if (!found){
      all_ingredients_found = false;
    }
    return true;
  }else{
    return false;
  }
}

void mix_cocktail(){

  // Decode the recipe
  /* all_ingredients_found = true;
  for(int i=0; i<4; i++){
    planned_amounts[i] = 0.0;
  }
  Recept recipe = Recept_init_zero;
  recipe.ingedients.funcs.decode = &decode_ingredient;

  // Create an input stream from buffer
  unsigned int buffer_length = server.arg("proto").length();

  if (buffer_length == 0){
    server.send(400,"text", "Empty Buffer.");
  }

  unsigned char buffer[buffer_length];
  server.arg("proto").getBytes(buffer,buffer_length);
  pb_istream_t input_stream = pb_istream_from_buffer(buffer,buffer_length);

  // Decode the next Cocktail Setup
  if (!pb_decode(&input_stream, Recept_fields, &recipe)){
    server.send(400,"text", "Error while decoding recipe.");
  }

  if (! all_ingredients_found){
    server.send(400,"text", "Not all ingredients found.");
  }*/
  
  float amounts[4] = {33, 33, 10, 4};
  long *durations = calculate_durations(amounts);

  for (int i=0; i < 4; i++){
    // Store the planned duration
    planned_pump_durations[i] = durations[i];

    // Start pumping
    pump_async(i, durations[i]);
    actual_pump_durations[i] = 0;
  }

  // Start the progress Ticker
  progress_ticker.attach_ms(500,update_progress);

  server.send(200,"text", String(planned_amounts[0]));
}

bool cocktail_in_make(){
  for (int i=0; i < 4; i++){
    /*if (pump_ticker[i].active()){
      return true;
    }*/
  }
  return false;
}

void progress(){
  String out = "";

  if (cocktail_in_make()){
    for(int i=0; i<4; i++){
      out+= String(actual_pump_durations[i]);
      out+= "/" + String(planned_pump_durations[i]) + "\n";
    }
  }else{
    out = "Nothing in progress. Let's make a new cocktail !";
  }
  
  server.send(200, "text", out);
}

long* calculate_durations(float* amount){
  int pump_ids[4] = {0,1,2,3};

  // Calculate the durations
  long durations[4];
  for (int i=0; i < 4; i++){
    durations[i] = (long)((amount[i] * 1000) / pump_speed[i]);
  }

  // Bubble sort the durations
  for (int i=3; i >=0; i--){
    for(int j=0; j <i; j++){
      if ( durations[j] > durations[j+1]){
        // swap values and indices 
        long temp = durations[j];
        int temp_idx = pump_ids[j];

        durations[j] = durations[j+1];
        pump_ids[j] = pump_ids[j+1];

        durations[j+1] = temp;
        pump_ids[j+1] = temp_idx;
      }
    }
  }

  // Correct the duration values
  static long fixed_durations[4];
  
  for (int i=0; i<4; i++){
    fixed_durations[i] = 0L;
  }
  
  for (int i=0; i <4; i++){
    long current_dur = durations[i];
    for(int j=i; j<4; j++){
      fixed_durations[pump_ids[j]] += current_dur / pump_speed_correction[i];
      durations[j] -= current_dur;
    }
  }

  return fixed_durations;
}

void welcome_screen(){
  // Display
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(0,0);
  tft.setTextSize(6);
  tft.println("Cocktail");
  tft.println("Maker");
  tft.setTextSize(2);
  tft.println("Familie Schneider");

  /*
  if (SD.begin(SD_CS_PIN)){
    tft.println("SD ok.");
  }{
    tft.println("SD failed.");
  }
  */
}

void setup() {
  welcome_screen();

  tft.drawRect(5,200,315,30, ILI9341_WHITE);

  // Initialize the pumps to be inactive
  for (int i=0; i<4; i++){
    pinMode(pump_pins[i], OUTPUT);
    digitalWrite(pump_pins[i], HIGH);
  }

  //Serial.begin(9600);

  // Connect to Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  // Wait for connection  
  Serial.println("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {   
    delay(500);
    //Serial.print(".");
    delay(500);
  }
  //Serial.println("Wifi connected.");
  //Serial.println(WiFi.localIP());

  // Rounting Table
  server.on("/setup", HTTP_POST, store_new_setup);
  server.on("/setup", HTTP_GET, display_current_setup);
  server.on("/make", HTTP_POST, mix_cocktail);
  server.on("/calibrate", HTTP_GET, calibrate);
  server.on("/progress", HTTP_GET, progress);


  // Begin Serving the REST interface
  server.collectHeaders(header_keys, 1);
  server.begin();
}

void loop() {
  server.handleClient();
}
