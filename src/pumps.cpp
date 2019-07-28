#include <pumps.h>
#include <Arduino.h>
#include <Wire.h>

PumpsControl::PumpsControl(){

  // Define the direction for the pumps
  this->inverted_pump_direction[0] = true;
  this->inverted_pump_direction[1] = false;
  this->inverted_pump_direction[2] = false;
  this->inverted_pump_direction[3] = false;
  this->inverted_pump_direction[4] = true;
  this->inverted_pump_direction[5] = true;
  this->inverted_pump_direction[6] = true;
  this->inverted_pump_direction[7] = true;


  // Define the pump speeds
  this->pump_speed[0] = 1.5;
  this->pump_speed[1] = 1.5;
  this->pump_speed[2] = 1.5;
  this->pump_speed[3] = 1.5;
  this->pump_speed[4] = 1.5;
  this->pump_speed[5] = 1.2;
  this->pump_speed[6] = 1.35;
  this->pump_speed[7] = 1.5;


  // Define the pump speed corrections
  this->pump_speed_correction[0] = 1.25/1.5;
  this->pump_speed_correction[1] = 1.35/1.5;
  this->pump_speed_correction[2] = 1.35/1.5;
  this->pump_speed_correction[3] = 1.5/1.5;
  this->pump_speed_correction[4] = 1.5/1.5;
  this->pump_speed_correction[5] = 1.5/1.5;
  this->pump_speed_correction[6] = 1.5/1.5;
  this->pump_speed_correction[7] = 1.5/1.5;
};

void PumpsControl::begin(){
  // Init the connection to the pump control chip
  this->mcp.begin();

  // Initialize the pumps to be inactive
  for (int i=0; i< 16; i++){
    this->mcp.pinMode(i, OUTPUT);
  }
};

void default_pump_off_callback(int pump_id){
  pumps_control.pump_off(pump_id);
};


void PumpsControl::pump_off(int pump_id){
  this->mcp.digitalWrite(2 * pump_id, LOW);
  this->mcp.digitalWrite(2 * pump_id + 1, LOW);
  this->pump_ticker[pump_id].detach();
}

void PumpsControl::pump_async(int pump_id, unsigned long duration){

  if (this->inverted_pump_direction[pump_id]){
    this->mcp.digitalWrite(2 * pump_id, HIGH);
    this->mcp.digitalWrite(2 * pump_id + 1, LOW);
  }else{
    this->mcp.digitalWrite(2 * pump_id, LOW);
    this->mcp.digitalWrite(2 * pump_id + 1, HIGH);
  }
  this->pump_ticker[pump_id].once_ms(duration, default_pump_off_callback, pump_id);
}

void PumpsControl::pump_backward(int pump_id, unsigned long duration){

  if (this->inverted_pump_direction[pump_id]){
    this->mcp.digitalWrite(2 * pump_id, LOW);
    this->mcp.digitalWrite(2 * pump_id + 1, HIGH);
  }else{
    this->mcp.digitalWrite(2 * pump_id, HIGH);
    this->mcp.digitalWrite(2 * pump_id + 1, LOW);
  }
  this->pump_ticker[pump_id].once_ms(duration, default_pump_off_callback, pump_id);
}

bool PumpsControl::pumps_in_use(){
  for (int i=0; i < NUMBER_OF_PUMPS; i++){
    if (this->pump_ticker[i].active()){
      return true;
    }
  }
  return false;
}

long* PumpsControl::calculate_durations(float* amounts){

  int pump_ids[NUMBER_OF_PUMPS] = {0,1,2,3,4,5,6,7};

  // Calculate the durations
  long durations[NUMBER_OF_PUMPS];
  for (int i=0; i < NUMBER_OF_PUMPS; i++){
    durations[i] = (long)((amounts[i] * 1000) / this->pump_speed[i]);
  }

  // Bubble sort the durations
  for (int i= NUMBER_OF_PUMPS - 1; i >=0; i--){
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
  static long fixed_durations[NUMBER_OF_PUMPS];

  for (int i=0; i< NUMBER_OF_PUMPS; i++){
    fixed_durations[i] = 0L;
  }

  for (int i=0; i < NUMBER_OF_PUMPS; i++){
    long current_dur = durations[i];
    for(int j=i; j< NUMBER_OF_PUMPS; j++){
      fixed_durations[pump_ids[j]] += current_dur / this->pump_speed_correction[i];
      durations[j] -= current_dur;
    }
  }

  return fixed_durations;

}
