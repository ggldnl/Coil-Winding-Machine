#ifndef CONFIG_HPP
#define CONFIG_HPP

// CNC Shield pinout
const byte STEPPER_1_STEP_PIN = 2;
const byte STEPPER_2_STEP_PIN = 3;

const byte STEPPER_1_DIR_PIN = 5;
const byte STEPPER_2_DIR_PIN = 6;

const byte ENABLE = 8;  // active-low (i.e. LOW turns on the drivers)

// Velocity and acceleration
const double MAX_VELOCITY_STEPS_S = 10000.0;
const double MIN_VELOCITY_STEPS_S = 500.0;
const double ACCELERATION = 5000.0;

// Homing
const double HOMING_VELOCITY_STEPS_S = 5000.0;
const long MAX_HOMING_STEPS = 1000000;

// Winding
const double WINDING_VELOCITY_STEPS_S = 1000.0;

// Limit switches
const uint8_t LIMIT_SWITCH_PIN = 10;

// Buttons
const uint8_t UP_BUTTON_PIN = 12;
const uint8_t DOWN_BUTTON_PIN = 11;
const uint8_t SELECT_BUTTON_PIN = 13;

// Reduction ratios
const int MICROSTEPPING = 8;
const float STEPS_PER_REVOLUTION = 200.0;
const float LEAD = 8.0;                                                      // leadscrew pitch, mm
const double STEPS_PER_MM = (STEPS_PER_REVOLUTION * MICROSTEPPING) / LEAD;    // steps/mm

// Constants
const float MAX_WIRE_DIAMETER = 2.0;
const float MAX_SPOOL_LENGTH = 50.0;
const float MAX_SPOOL_DIAMETER = 20.0;
const float MAX_LAYER_COUNT = 5.0;

const float MIN_WIRE_DIAMETER = 0.25;
const float MIN_SPOOL_LENGTH = 5.0;
const float MIN_SPOOL_DIAMETER = 6.0;
const float MIN_LAYER_COUNT = 1.0;

const float DELTA_WIRE_DIAMETER = 0.01;
const float DELTA_SPOOL_LENGTH = 1.0;
const float DELTA_SPOOL_DIAMETER = 1.0;
const float DELTA_LAYER_COUNT = 1.0;

const float BIG_DELTA_WIRE_DIAMETER = 0.1;
const float BIG_DELTA_SPOOL_LENGTH = 5.0;
const float BIG_DELTA_SPOOL_DIAMETER = 5.0;
const float BIG_DELTA_LAYER_COUNT = 5.0;

const float MIN_TIME = 10;
const float MAX_TIME = 120;

const float DELTA_TIME = 1;
const float DELTA_SPEED = 100;

const float BIG_DELTA_TIME = 10;
const float BIG_DELTA_SPEED = 1000;

// States and events
const uint8_t STATE_MENU_SPLASH_SCREEN = 0;
const uint8_t STATE_ERROR = 1;

const uint8_t STATE_WIND = 2;
const uint8_t STATE_SET_WIRE_DIAMETER = 3;
const uint8_t STATE_SET_SPOOL_LENGTH = 4;
const uint8_t STATE_SET_SPOOL_DIAMETER = 5;
const uint8_t STATE_SET_LAYER_COUNT = 6;
const uint8_t STATE_WIND_ASK_CONFIRM = 7;
const uint8_t STATE_START_WINDING = 8;

const uint8_t STATE_UNWIND = 9;
const uint8_t STATE_SET_TIME = 10;
const uint8_t STATE_SET_SPEED = 11;
const uint8_t STATE_SET_DIRECTION = 12;
const uint8_t STATE_UNWIND_ASK_CONFIRM = 13;
const uint8_t STATE_START_UNWINDING = 14;

const uint8_t EVENT_TIMEOUT = 21;
const uint8_t EVENT_UP_PRESS = 22;
const uint8_t EVENT_UP_LONGPRESS = 23;
const uint8_t EVENT_DOWN_PRESS = 24;
const uint8_t EVENT_DOWN_LONGPRESS = 25;
const uint8_t EVENT_SELECT_PRESS = 26;
const uint8_t EVENT_SELECT_LONGPRESS = 27;
const uint8_t EVENT_UPDATE_PROGRESS = 28;
const uint8_t EVENT_RESET = 29;

#endif // CONFIG_HPP