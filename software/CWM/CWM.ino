#include "logger.hpp"
#include "button.hpp"
#include "stepper.hpp"
#include "config.hpp"
#include "automaton.hpp"
#include "states.hpp"

// Define the functions
void homeAxis(StepperMotor&, Button&, long, double = HOMING_VELOCITY_STEPS_S);
void homeAll();
void moveAll();
void disable();
void enable();

// Automaton instance
FiniteStateAutomaton fsm;

// Define the steppers
StepperMotor stepperCoil(STEPPER_1_STEP_PIN, STEPPER_1_DIR_PIN);
StepperMotor stepperFeeder(STEPPER_2_STEP_PIN, STEPPER_2_DIR_PIN);

// Define the buttons
Button lowerLimitSwitch(LOWER_LIMIT_SWITCH_PIN);
Button upperLimitSwitch(UPPER_LIMIT_SWITCH_PIN);
Button upButton(UP_BUTTON_PIN);
Button downButton(DOWN_BUTTON_PIN);
Button selectButton(SELECT_BUTTON_PIN);

// Variables
int wireGauge = MIN_WIRE_GAUGE;
int spoolLength = MIN_SPOOL_LENGTH;
int spoolDiameter = MIN_SPOOL_DIAMETER;
int layerCount = MIN_LAYER_COUNT;
bool startWinding = 0;

/* ---------------------------------- Setup --------------------------------- */

void setup() {

  delay(2000);

  // Initialize serial communication
  Serial.begin(115200);

  // Set log level to INFO
  Logger::setLogLevel(Logger::DEBUG);

  // Setup the LCD
  setupLCD();

  // Create and add states
  fsm.addState(new StateMenuSplashScreen(&fsm));
  fsm.addState(new StateSetWireGauge(&fsm, wireGauge));
  fsm.addState(new StateSetSpoolLength(&fsm, spoolLength));
  fsm.addState(new StateSetSpoolDiameter(&fsm, spoolDiameter));
  fsm.addState(new StateSetLayerCount(&fsm, layerCount));
  fsm.addState(new StateStartWinding(&fsm, startWinding));

  // Start the automaton with the first menu item
  fsm.start(STATE_MENU_SPLASH_SCREEN);

  /*
  // Enable the board
  pinMode(ENABLE, OUTPUT);
  enable();
  
  // Home axis
  homeAll();
  Logger::info("All axis homed.");
  */

  delay(2000);
  fsm.onEvent(EVENT_TIMEOUT);
}

/* -------------------------------- Movement -------------------------------- */

void wind() {
    for (int i = 0; i <= 100; i += 1) {
      Logger::debug("Winding {} %", i);
      fsm.onEvent(EVENT_UPDATE_PROGRESS);
      delay(50); // Delay for demonstration
    }
}

void moveAll() {
  /**
   * Move all steppers until they reach the target position.
   */
  while (!(stepperCoil.isAtTarget() && stepperFeeder.isAtTarget())) {
    stepperCoil.step();
    stepperFeeder.step();
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
  homeAxis(stepperFeeder, lowerLimitSwitch, MAX_HOMING_STEPS);
  Logger::debug("Axis 0 homed.");

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

    if (upButton.pressed()) {
        // logger.debug("Up button pressed");
        fsm.onEvent(EVENT_UP_PRESS);
    }
    if (upButton.isHeld()) {
        // logger.debug("Up button long pressed");
        fsm.onEvent(EVENT_UP_LONGPRESS);
    }
    if (downButton.pressed()) {
        // logger.debug("Down button pressed");
        fsm.onEvent(EVENT_DOWN_PRESS);
    }
    if (downButton.isHeld()) {
        // logger.debug("Down button long pressed");
        fsm.onEvent(EVENT_DOWN_LONGPRESS);
    }
    if (selectButton.pressed()) {
        // logger.debug("Select button pressed");
        fsm.onEvent(EVENT_SELECT_PRESS);
    }
    if (selectButton.isHeld()) {
        // logger.debug("Select button long pressed");
        fsm.onEvent(EVENT_SELECT_LONGPRESS);
    }

    if (startWinding) {
        // Start the winding routine (blocking)
        wind();
        fsm.onEvent(EVENT_RESET);
        startWinding = false;        
    }

    Logger::debug("{}, {}, {}, {}, {}", wireGauge, spoolLength, spoolDiameter, layerCount, startWinding);
}
