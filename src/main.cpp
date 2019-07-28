#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <recept.pb.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <Ticker.h>
#include <SPI.h>

#include <cobs/Stream.h>
#include <pb_arduino.h>
#include <string_stream.h>

#include <Adafruit_ILI9341.h>
#include <pumps.h>
#include <display.h>
#include <frames/welcome.h>
#include <frames/clock.h>
#include <frames/i2cscan.h>
#include <frames/setup.h>
#include <FS.h>
#include <JPEGDecoder.h>
#include <rendering.h>
#include <TaskScheduler.h>

PumpsControl pumps_control;
DisplayManager display_manager;

// Multitasking
void wait_for_wifi_connection();
Task wifi_init_task(500, TASK_FOREVER, & wait_for_wifi_connection);
Scheduler scheduler;

// Time Keeping
#include <WifiUDP.h>
#include <NTPClient.h>
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, "europe.pool.ntp.org", 7200);

#define SDA D1
#define SCL D2
#define TFT_CS D3
#define IR_SENSOR_PIN D4
#define TFT_DC D8

// IR Sensor
void ICACHE_RAM_ATTR ir_isr();
int current_ir_state = 0;

// Touch Screen
#include <XPT2046_Touchscreen.h>
XPT2046_Touchscreen touch(D0);

// Wifi setup
const char *ssid = "Hagelschlag";
const char *password = "12121212";

// REST server
ESP8266WebServer server;
const char* header_keys[1] = {"content-type"};

// The current cocktail machine setup
CocktailSetup cocktail_setup = CocktailSetup_init_zero;

//PumpsControl p_control;
void welcome_screen();
void pump_off_callback(int);
bool cocktail_in_make();


Ticker progress_ticker;
Ticker done_ticker;

long planned_pump_durations[NUMBER_OF_PUMPS] = {0,0,0,0,0,0,0,0};
long actual_pump_durations[NUMBER_OF_PUMPS] = {0,0,0,0,0,0,0,0};

float planned_amounts[NUMBER_OF_PUMPS] = {0,0,0,0,0,0,0,0};
bool all_ingredients_found = true;

// Display setup
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

WelcomeFrame welcome_frame = WelcomeFrame(&tft);
ClockFrame clock_frame = ClockFrame(&ntpClient ,&tft);
I2CScanFrame i2c_frame = I2CScanFrame(&tft);
SetupFrame setup_frame = SetupFrame(&display_manager, &tft);
//Adafruit_ImageReader img_reader;


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

  display_manager.setFrame(&setup_frame);
}

void display_current_setup(){

  if (server.header(1).startsWith("application/x-")){
    unsigned char buffer[1024];
    pb_ostream_t out = pb_ostream_from_buffer(buffer, 1024);
    pb_encode(&out, CocktailSetup_fields, &cocktail_setup);

    server.send_P(200, "text", (const char*)buffer, 1024);
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
    display_manager.setFrame(&setup_frame);
  }
}

void write_dummy_setup(){
  cocktail_setup.liquids[0].pump_id = 0;
  strcpy(cocktail_setup.liquids[0].name, "Water 1");
  cocktail_setup.liquids_count++;

  cocktail_setup.liquids[1].pump_id = 1;
  strcpy(cocktail_setup.liquids[1].name, "Water 2");
  cocktail_setup.liquids_count++;

  cocktail_setup.liquids[2].pump_id = 2;
  strcpy(cocktail_setup.liquids[2].name, "Water 3");
  cocktail_setup.liquids_count++;

  cocktail_setup.liquids[3].pump_id = 3;
  strcpy(cocktail_setup.liquids[3].name, "Water 4");
  cocktail_setup.liquids_count++;

  cocktail_setup.liquids[4].pump_id = 4;
  strcpy(cocktail_setup.liquids[4].name, "Water 5");
  cocktail_setup.liquids_count++;

  cocktail_setup.liquids[5].pump_id = 5;
  strcpy(cocktail_setup.liquids[5].name, "Water 6");
  cocktail_setup.liquids_count++;

  cocktail_setup.liquids[6].pump_id = 6;
  strcpy(cocktail_setup.liquids[6].name, "Water 7");
  cocktail_setup.liquids_count++;

  cocktail_setup.liquids[7].pump_id = 7;
  strcpy(cocktail_setup.liquids[7].name, "Water 8");
  cocktail_setup.liquids_count++;
}

void calibrate(){
  int number_of_pumps = server.arg("pump").toInt();
  long duration = server.arg("duration").toInt();

  for (int i= 0; i < number_of_pumps; i++){
    pumps_control.pump_async(i, duration);
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
        tft.println("-> " + String(ing.name) + " found: " + String(ing.amount));


        // add the amount to the plan
        planned_amounts[i] = ing.amount;
      }
    }

    if (!found){
      all_ingredients_found = false;

      tft.println("-> " + String(ing.name) + "not found.");
    }
    return true;
  }else{
    return false;
  }
}

void mix_cocktail(){

  // clear the tft screen
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,0);
  tft.setTextSize(2);
  tft.println("Mix new cocktail:");

  if (current_ir_state != HIGH){

    tft.println();
    tft.setTextColor(ILI9341_RED);
    tft.println("No glas detected");
    server.send(400,"text", "No glas detected.");
    return;
  }


  // Decode the recipe
  all_ingredients_found = true;
  for(int i=0; i< NUMBER_OF_PUMPS; i++){
    planned_amounts[i] = 0.0;
  }
  Recept recipe = Recept_init_zero;
  recipe.ingedients.funcs.decode = &decode_ingredient;

  // Create an input stream from buffer
  unsigned int buffer_length = server.arg("proto").length();

  if (buffer_length == 0){
    server.send(400,"text", "Empty Buffer.");
    tft.println("Empty Buffer.");
    return;
  }

  //unsigned char buffer[buffer_length];
  //server.arg("proto").getBytes(buffer,buffer_length);
  //tft.println(server.arg("proto"));
  //return;

  StringStream stream = StringStream(server.arg("proto"));
  packetio::COBSStream cstream(stream);
  pb_istream_s input_stream = as_pb_istream(cstream);

  // Decode the next Cocktail Setup
  if (!pb_decode(&input_stream, Recept_fields, &recipe)){
    //server.send(400,"text", "Error while decoding recipe.");
    //tft.println("Error while decoding recipe.");
  }

  // Print the Cocktail name
  tft.println();
  tft.setTextColor(ILI9341_YELLOW);
  tft.println(recipe.name);
  tft.setTextColor(ILI9341_WHITE);
  tft.println();

  if (! all_ingredients_found){
    server.send(400,"text", "Not all ingredients found.");
    tft.println("Not all ingredients found.");
    return;
  }

  for (int i=0; i < cocktail_setup.liquids_count; i++){
    String out= "";
    out += "Pump ";
    out += String(cocktail_setup.liquids[i].pump_id);
    out += ": ";
    out += planned_amounts[i];
    tft.println(out);
  }


  // Compute the corresponding pump durations
  //long *durations = calculate_durations(planned_amounts);
  long* durations = pumps_control.calculate_durations(planned_amounts);

  delay(5000);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,0);

  for (int i=0; i < NUMBER_OF_PUMPS; i++){
    // Store the planned duration
    planned_pump_durations[i] = durations[i];

    if (planned_pump_durations[i] > 0){
      tft.setCursor(25, 2 *i * 20);
      tft.print(cocktail_setup.liquids[i].name);
      tft.drawRect(20, (2*i + 1) * 20, 280, 15, ILI9341_BLUE);
    }

    // Start pumping
    pumps_control.pump_async(i, durations[i]);
    actual_pump_durations[i] = 0;
    delay(750);
  }

  // Start the progress Ticker
  progress_ticker.attach_ms(500,update_progress);

  server.send(200,"text", String(planned_amounts[0]));
}

bool cocktail_in_make(){
  return pumps_control.pumps_in_use();
}

void progress(){
  String out = "";

  if (cocktail_in_make()){
    for(int i=0; i< NUMBER_OF_PUMPS; i++){
      out+= String(actual_pump_durations[i]);
      out+= "/" + String(planned_pump_durations[i]) + "\n";

      if (planned_pump_durations[i] > 0 && actual_pump_durations[i] % 3000 == 0){
        tft.fillRect(20, (2*i + 1) * 20, int(280 * actual_pump_durations[i] / planned_pump_durations[i]), 15, ILI9341_BLUE);
      }
    }
  }else{
    bool finished = false;
    for(int i=0; i< NUMBER_OF_PUMPS; i++){
      out+= String(actual_pump_durations[i]);
      out+= "/" + String(planned_pump_durations[i]) + "\n";

      if (planned_pump_durations[i] > 0){
        tft.fillRect(20, (2*i + 1) * 20, 280, 15, ILI9341_BLUE);
        finished = true;
      }
    }

    out = "Nothing in progress. Let's make a new cocktail !";
    server.send(201, "text", out);

    if (finished){
      tft.setCursor(40,180);
      tft.setTextSize(3);
      tft.println("Finished");
    }

    delay(3000);
    welcome_screen();
  }

  server.send(200, "text", out);
}

void welcome_screen(){
  Serial.println("Welcome");
  display_manager.setFrame(&welcome_frame);
}

void ir_isr(){
  int value = digitalRead(IR_SENSOR_PIN);
  String answer;
  if (value == HIGH){
    answer = "Glas detected. " + String(value);
  }else{

    answer = "Nothing detected. " + String(value);
  }

  if (value != current_ir_state){
    Serial.println(answer);

    if (value == LOW && pumps_control.pumps_in_use()){
      for(int i =0; i < 8; i++){
        pumps_control.pump_off(i);
      }
    }
  }

  current_ir_state = value;
}

void ir_sensor(){

  // Clear the TFT screen
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,0);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);

  // Read the sensor value
  int value = digitalRead(IR_SENSOR_PIN);
  String answer;

  if (value == HIGH){
    answer = "Glas detected. " + String(value);
  }else{

    answer = "Nothing detected. " + String(value);
  }

  tft.println(answer);
  server.send(200, "text", answer);
}

void clean(){
  // Clear the tft
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(8,0);
  tft.setTextSize(6);
  tft.println("Cleaning");

  for(int i=0; i < NUMBER_OF_PUMPS; i++){
    pumps_control.pump_async(i, 20000);
    delay(750);
  }
  tft.setTextSize(2);
  tft.println("started.");

  server.send(200,"text", "ok");
  //done_ticker.once_ms(20000, done_callback);
}

void startup(){
  // Clear the tft
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(8,0);
  tft.setTextSize(3);
  tft.println("Startup");
  tft.println("Adjustment");

  for(int i=0; i < NUMBER_OF_PUMPS; i++){
    pumps_control.pump_async(i, 3000);
    delay(750);
  }
  tft.setTextSize(2);
  tft.println("started.");

  server.send(200,"text", "ok");
  //done_ticker.once_ms(3000, done_callback);
}

void show_time(){
  clock_frame.begin();
  clock_frame.update();
  server.send(200,"text", "ok");
}

void i2c_scan(){
  i2c_frame.begin();
  i2c_frame.update();
  server.send(200,"text", "ok");
}

void shake(){

  for(int i =0; i <2; i++){
    pumps_control.pump_async(1,5000);
    delay(6000);
    pumps_control.pump_backward(1,5000);
    delay(6000);
  }

  server.send(200,"text", "ok");
}

void setup() {

  // Chip Select for SPI communication
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);

  pinMode(D4, OUTPUT);
  digitalWrite(D4, LOW);

  //pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN), ir_isr, CHANGE);
  //current_ir_state = digitalRead(IR_SENSOR_PIN);

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("TFT Screen initialized.");


  // Initialize the Touch Screen
  if (touch.begin()){
    touch.setRotation(3);
    tft.println("Touch initialized.");
  }else{
    tft.setTextColor(ILI9341_RED);
    tft.println("Touch failed.");
    tft.setTextColor(ILI9341_WHITE);
  }

  // Try to init the file system
  if (! SPIFFS.begin()){
    tft.setTextColor(ILI9341_RED);
    tft.println("FS failed.");
    tft.setTextColor(ILI9341_WHITE);
  }else{
    tft.println("FS initialized.");
  }

  delay(3000);


  welcome_screen();
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(20,130);

  Serial.begin(9600);
  delay(500);


  // Connect to Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  scheduler.addTask(wifi_init_task);
  wifi_init_task.enable();

  write_dummy_setup();

  // Initialize SDA, SCL
  Wire.begin(SDA, SCL);
  pumps_control.begin();
}

void onWiFiConnected(){

  // Rounting Table
  server.on("/setup", HTTP_POST, store_new_setup);
  server.on("/setup", HTTP_GET, display_current_setup);
  server.on("/make", HTTP_POST, mix_cocktail);
  server.on("/calibrate", HTTP_GET, calibrate);
  server.on("/progress", HTTP_GET, progress);
  server.on("/startup", HTTP_GET, startup);
  server.on("/clean", HTTP_GET, clean);
  server.on("/ir", HTTP_GET, ir_sensor);
  server.on("/time", HTTP_GET, show_time);
  server.on("/i2c", HTTP_GET, i2c_scan);
  server.on("/shake", HTTP_GET, shake);

  // Initialize the time client
  ntpClient.begin();

  // Begin Serving the REST interface
  server.collectHeaders(header_keys, 1);
  server.begin();

  wifi_init_task.disable();
}

void wait_for_wifi_connection(){
  if(WiFi.status() != WL_CONNECTED) {
    yield();
    return;
  }else{
    wifi_init_task.yield(&onWiFiConnected);
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED){
    server.handleClient();
  }

  if (touch.touched()){
    // Delegate the Touch to Display Manager
    TS_Point tp = touch.getPoint();
    display_manager.onTouchEvent(tp);
  }

  display_manager.update();
  scheduler.execute();
}
