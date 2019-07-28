#ifndef PUMPS_H
#define PUMPS_H
#define NUMBER_OF_PUMPS 8

#include <Ticker.h>
#include <Adafruit_MCP23017.h>

class PumpsControl{



public:
  PumpsControl();
  void begin();
  void pump_off(int pump_id);
  long* calculate_durations(float* amounts);
  void pump_async(int pump_id, unsigned long duration);
  void pump_backward(int pump_id, unsigned long duration);
  bool pumps_in_use();

private:
  Ticker pump_ticker[NUMBER_OF_PUMPS];
  Adafruit_MCP23017 mcp;

  long planned_pump_durations[NUMBER_OF_PUMPS]= {0,0,0,0,0,0,0,0};
  long actual_pump_durations[NUMBER_OF_PUMPS]= {0,0,0,0,0,0,0,0};

  // Individual pump speed values from calibration
  float pump_speed[NUMBER_OF_PUMPS];//1.5245108352619399;
  float pump_speed_correction[NUMBER_OF_PUMPS] = {1.25/1.5, 1.35/1.5, 1.35/1.5, 1.5/1.5};

  bool inverted_pump_direction[NUMBER_OF_PUMPS];

};

extern PumpsControl pumps_control;

#endif
