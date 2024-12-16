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

// Winding
float wireDiameter = 0.2;   // mm
float spoolLength = 41;     // mm
float spoolDiameter = 14;    // mm
float layerCount = 1;

// Unwinding
float time = 10;              // s
float speed = 5000.0;         // steps/s
bool direction = 0;

int state = 0;            // 0 idle, 1 winding, 2 unwinding

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

  fsm.addState(new StateWind(&fsm));
  fsm.addState(new StateSetWireDiameter(&fsm, wireDiameter));
  fsm.addState(new StateSetSpoolLength(&fsm, spoolLength));
  fsm.addState(new StateSetSpoolDiameter(&fsm, spoolDiameter));
  fsm.addState(new StateSetLayerCount(&fsm, layerCount));
  fsm.addState(new StateWindAskConfirm(&fsm));
  fsm.addState(new StateStartWinding(&fsm, state));

  fsm.addState(new StateUnwind(&fsm));
  fsm.addState(new StateSetTime(&fsm, time));
  fsm.addState(new StateSetSpeed(&fsm, speed));
  fsm.addState(new StateSetDirection(&fsm, direction));
  fsm.addState(new StateUnwindAskConfirm(&fsm));
  fsm.addState(new StateStartUnwinding(&fsm, state));

  // Start the automaton with the first menu item
  fsm.start(STATE_MENU_SPLASH_SCREEN);

  // Enable the board
  enable();

  // Home axis
  home();
  // Logger::debug("All axis homed"); 

  // Temporary disable the board
  disable();

  // Move to the menu
  // delay(2000);
  fsm.onEvent(EVENT_TIMEOUT);
}

/* -------------------------------- Movement -------------------------------- */

void wind() {

    /*
    Logger::debug("Starting winding process");
    Logger::debug("Wire diameter: {}", wireDiameter);
    Logger::debug("Spool length: {}", spoolLength);
    Logger::debug("Spool diameter: {}", spoolDiameter);
    Logger::debug("Layer count: {}", layerCount);
    */

    for (int layer = 0; layer < layerCount; ++layer) {

        // Logger::debug("---------");

        // Current diameter of the spool at this layer
        double currentDiameter = spoolDiameter + 2 * layer * wireDiameter;
        // Logger::debug("Layer {}: Current diameter: {}", layer, currentDiameter);

        // Length of wire wound per full revolution of the spool motor
        double wireWoundPerRev = PI * currentDiameter;
        // Logger::debug("Layer {}: Wire wound per revolution: {} mm", layer, wireWoundPerRev);

        // Number of wire revolutions around the coil
        double numRevolutions = spoolLength / wireDiameter;
        // Logger::debug("Layer {}: Num revolutions: {}", layer, numRevolutions);

        // Number of steps for the coil motor to perform all the revolutions
        long totalCoilSteps = numRevolutions * STEPS_PER_REVOLUTION * MICROSTEPPING;
        // Logger::debug("Layer {}: Total coil steps: {}", layer, totalCoilSteps);

        // Time the coil motor takes to perform a single revolution
        double timeRevolution = (STEPS_PER_REVOLUTION * MICROSTEPPING) / WINDING_VELOCITY_STEPS_S;
        // Logger::debug("Layer {}: Time to complete a revolution: {}", layer, timeRevolution);

        // The coil motor has a fixed velocity. We want the feeder to move by wireDiameter mm 
        // for each full revolution of the coil motor. We know how many steps it takes to perform
        // wireDiameter mm. We impose the same time as the time needed to the coil motor to perform
        // a single revolution and we compute the feeder velocity from it. 
        long feederVelocity = (STEPS_PER_MM * wireDiameter) / timeRevolution;
        // Logger::debug("Layer {}: Feeder velocity: {}", layer, feederVelocity);

        // Compute the total steps needed to the feeder
        long totalFeederSteps = STEPS_PER_MM * spoolLength;
        // Logger::debug("Layer {}: Feeder steps: {}", layer, totalFeederSteps);

        // Move both motors simultaneously for the current layer
        stepperCoil.moveToPosition(totalCoilSteps, WINDING_VELOCITY_STEPS_S);
        stepperFeeder.moveToPosition(-totalFeederSteps, feederVelocity);

        moveAll();
    }

    // Logger::debug("Winding process complete");
}

void unwind() {

    /*
    Logger::debug("Starting winding process");
    Logger::debug("Time: {}", time);
    Logger::debug("Speed: {}", speed);
    Logger::debug("Direction: {}", direction ? "Forward" : "Backward");
    */

    Logger::debug("Time: {}", time);
    Logger::debug("Speed: {}", speed);
    Logger::debug("Direction: {}", direction ? "Forward" : "Backward");

    // speed = distance / time -> distance = speed * time
    int distance = speed * time;

    stepperFeeder.moveToPosition(0, speed);
    stepperCoil.moveToPosition(distance, speed);

    moveAll();

    Logger::debug("Unwinding process complete");

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
   * The motor stops after #homingSteps anyway. We can't use a trapezoidal speed profile
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
  // Logger::debug("Axis 0 homed.");

}

void enable() {
  digitalWrite(ENABLE, LOW);
  // Logger::debug("Steppers enabled.");
}

void disable() {
  digitalWrite(ENABLE, HIGH);
  // Logger::debug("Steppers disabled.");
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

    switch (state) {
        case 1:

            enable();

            // Start the winding routine (blocking)
            wind();

            // Done
            fsm.onEvent(EVENT_RESET);
            state = 0;

            // Disable the board
            disable(); 

            break;

        case 2:

            enable();

            // Start the unwinding routine (blocking)
            unwind();

            // Done
            fsm.onEvent(EVENT_RESET);
            state = 0;

            // Disable the board
            disable();

            break;

        default:
            break;
    }
}
