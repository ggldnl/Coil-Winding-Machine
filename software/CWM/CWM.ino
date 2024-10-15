#include "logger.hpp"
#include "button.hpp"
#include "stepper.hpp"
#include "config.hpp"


void homeAxis(StepperMotor&, Button&, long, double = HOMING_VELOCITY_STEPS_S);
void homeAll();
void moveAll();
void disable();
void enable();

// Define the steppers
StepperMotor stepper1(STEPPER_1_STEP_PIN, STEPPER_1_DIR_PIN);
StepperMotor stepper2(STEPPER_2_STEP_PIN, STEPPER_2_DIR_PIN);

// Define the buttons
Button lowerLimitSwitch(LOWER_LIMIT_SWITCH_PIN);
Button upperLimitSwitch(UPPER_LIMIT_SWITCH_PIN);


void setup() {

  delay(2000);

  // Initialize serial communication
  Serial.begin(115200);

  // Set log level to INFO
  Logger::setLogLevel(Logger::DEBUG);

  // Enable the board
  pinMode(ENABLE, OUTPUT);
  enable();
  
  // Home axis
  homeAll();
  Logger::info("All axis homed.");

  delay(2000);
  disable();
}

void moveAll() {
  /**
   * Move all steppers until they reach the target position.
   */
  while (!(stepper1.isAtTarget() && stepper2.isAtTarget())) {
    stepper1.step();
    stepper2.step();
  }
}

/* --------------------------------- Homing --------------------------------- */

void homeAxis(
    StepperMotor& stepper, 
    Button& limitSwitch, 
    long homingSteps, 
    double velocity
  ) {
  /**
   * Home the specified axis by moving the motor with constant speed until button fires.
   * The motor stops after #homingSteps anyway. We can't use a tapezoidal speed profile
   * since we don't know in advance how many steps we need to take and thus we cannot
   * decelerate properly. We could gradually accelerate and until we reach the maximum
   * velocity but I prefer to cruise to the limit switch with minimum velocity in order 
   * to move safely to the target.
   */

  stepper.moveToPosition(homingSteps, velocity);
  while (!stepper.isAtTarget()) {

    if (limitSwitch.pressed()) {
      Logger::debug("Endstop {} reached.", limitSwitch.getPin());
      break;
    }

    // Perform a step with constant velocity
    stepper.step();

    // Little delay to smooth things out
    delayMicroseconds(10);
  }

  // Set the zero
  stepper.setCurrentPosition(0);
}

void homeAll() {
  /**
   * Home all the axis.
   */

  // Move the first axis down for 10000 steps or until the limit switch registers a press
  homeAxis(stepper1, button1, -MAX_HOMING_STEPS);
  Logger::debug("Axis 0 homed.");

  // homeAxis(stepper2, button2);
  homeAxis(stepper2, button2, MAX_HOMING_STEPS);
  Logger::debug("Axis 1 homed.");
}

void enable() {
  digitalWrite(ENABLE, LOW);
  Logger::debug("Steppers enabled.");
}

void disable() {
  digitalWrite(ENABLE, HIGH);
  Logger::debug("Steppers disabled.");
}

/* ---------------------------------- Loop ---------------------------------- */

void loop() {
  // Nothing to do here
}
