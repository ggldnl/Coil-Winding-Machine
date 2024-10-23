#include "logger.hpp"
#include "button.hpp"
#include "stepper.hpp"
#include "config.hpp"
#include "automaton.hpp"
#include "states.hpp"

// Define the functions
void homeAxis(StepperMotor&, Button&, long, double);
void home();
void moveAll();
void disable();
void enable();

// Automaton instance
FiniteStateAutomaton fsm;

// Define the steppers
StepperMotor stepperCoil(STEPPER_1_STEP_PIN, STEPPER_1_DIR_PIN);
StepperMotor stepperFeeder(STEPPER_2_STEP_PIN, STEPPER_2_DIR_PIN);

// Define the buttons
Button limitSwitch(LIMIT_SWITCH_PIN);
Button upButton(UP_BUTTON_PIN);
Button downButton(DOWN_BUTTON_PIN);
Button selectButton(SELECT_BUTTON_PIN);

// Variables
/*
float wireDiameter = MIN_WIRE_DIAMETER;
float spoolLength = MIN_SPOOL_LENGTH;
float spoolDiameter = MIN_SPOOL_DIAMETER;
float layerCount = MIN_LAYER_COUNT;
*/

float wireDiameter = 0.5;   // mm
float spoolLength = 41;     // mm
float spoolDiameter = 9;    // mm
float layerCount = 1;

bool winding = 0;

/* ---------------------------------- Setup --------------------------------- */

void setup() {

  delay(2000);

  // Initialize serial communication
  Serial.begin(115200);

  // Set log level to INFO
  Logger::setLogLevel(Logger::DEBUG);

  // Setup the board
  pinMode(ENABLE, OUTPUT);
  disable();

  // Setup the LCD
  setupLCD();

  // Create and add states
  fsm.addState(new StateMenuSplashScreen(&fsm));
  fsm.addState(new StateSetWireDiameter(&fsm, wireDiameter));
  fsm.addState(new StateSetSpoolLength(&fsm, spoolLength));
  fsm.addState(new StateSetSpoolDiameter(&fsm, spoolDiameter));
  fsm.addState(new StateSetLayerCount(&fsm, layerCount));
  fsm.addState(new StateAskConfirm(&fsm));
  fsm.addState(new StateStartWinding(&fsm, winding));

  // Start the automaton with the first menu item
  fsm.start(STATE_MENU_SPLASH_SCREEN);

  // Enable the board
  enable();

  // Home axis
  home();
  Logger::info("All axis homed.");

  // Temporary disable the board
  disable();

  // Move to the menu
  // delay(2000);
  fsm.onEvent(EVENT_TIMEOUT);
}

/* -------------------------------- Movement -------------------------------- */

void wind() {

    Logger::debug("Starting winding process");
    Logger::debug("Wire diameter: {}", wireDiameter);
    Logger::debug("Spool length: {}", spoolLength);
    Logger::debug("Spool diameter: {}", spoolDiameter);
    Logger::debug("Layer count: {}", layerCount);

    for (int layer = 0; layer < layerCount; ++layer) {

        Logger::debug("---------");

        // Current diameter of the spool at this layer
        double currentDiameter = spoolDiameter + 2 * layer * wireDiameter;
        Logger::debug("Layer {}: Current diameter: {}", layer, currentDiameter);

        // Length of wire wound per full revolution of the spool motor
        double wireWoundPerRev = PI * currentDiameter;
        Logger::debug("Layer {}: Wire wound per revolution: {} mm", layer, wireWoundPerRev);

        // Feeder moves by wire's diameter per revolution
        double feederDistancePerRev = wireDiameter;
        Logger::debug("Layer {}: Feeder distance per revolution: {} mm", layer, feederDistancePerRev);

        // Ratio between the two motors
        double stepRatio = feederDistancePerRev / wireWoundPerRev;
        Logger::debug("Layer {}: Step ratio: {}", layer, stepRatio);

        long totalFeederSteps = spoolLength * STEPS_PER_MM;
        Logger::debug("Layer {}: Total feeder steps: {}", layer, totalFeederSteps);

        // Calculate total coil and feeder steps for the current layer
        long totalCoilSteps = totalFeederSteps / stepRatio;
        Logger::debug("Layer {}: Total coil steps: {}", layer, totalCoilSteps);

        // Scale the spool motor's velocity
        double feederTime = totalFeederSteps / (STEPS_PER_MM * WINDING_VELOCITY_STEPS_S);
        double velocityCoil = totalCoilSteps / feederTime;

        // Move both motors simultaneously for the current layer
        stepperCoil.moveToPosition(totalCoilSteps, velocityCoil);
        stepperFeeder.moveToPosition(-totalFeederSteps, WINDING_VELOCITY_STEPS_S);

        moveAll();
    }

    Logger::debug("Winding process complete");
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

void home() {
  /**
   * Home all the axis.
   */

  // Move the first axis down for 10000 steps or until the limit switch registers a press
  homeAxis(stepperFeeder, limitSwitch, MAX_HOMING_STEPS, HOMING_VELOCITY_STEPS_S);
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

    if (winding) {

        enable();

        // Start the winding routine (blocking)
        wind();

        // Done
        fsm.onEvent(EVENT_RESET);
        winding = false;       

        // Disable the board
        disable(); 
    }
}
