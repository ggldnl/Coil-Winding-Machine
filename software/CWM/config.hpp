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

// Homing and limit switches
const double HOMING_VELOCITY_STEPS_S = 5000.0;
const long MAX_HOMING_STEPS = 1000000;

// Limit switches
const byte LOWER_LIMIT_SWITCH_PIN = 9;
const byte UPPER_LIMIT_SWITCH_PIN = 10;

// Reduction ratios
const int MICROSTEPPING = 16;
const float STEPS_PER_REVOLUTION = 200.0;
const float LEAD = 8.0;                                                                 // leadscrew pitch, mm
const double STEPS_PER_M = (STEPS_PER_REVOLUTION * MICROSTEPPING) / (LEAD / 1000.0);    // steps/m

#endif // CONFIG_HPP