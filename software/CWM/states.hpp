#include "automaton.hpp"

// The only I didn't end up writing
#include <LiquidCrystal_I2C.h>


// LCD settings
LiquidCrystal_I2C lcd(0x27, 16, 2);

class StateWithFloat : public State {
/**
 * This class represents a state with a float variable that can be incremented 
 * and decremented in a fixed range.  
 */

public:
    StateWithFloat(uint8_t id, FiniteStateAutomaton* automaton, float& externalVar, float minVal, float maxVal)
        : State(id, automaton), _externalVar(externalVar), _init(externalVar), _min(minVal), _max(maxVal), _hasChanged(false) {
          _externalVar = min(_externalVar, _max);
          _externalVar = max(_externalVar, _min);
        }

    void increment(float inc) {
        if (_externalVar < _max) {
            _externalVar += inc;
            if (_externalVar > _max) {
                _externalVar = _max;  // Clamp to max
            }
            _hasChanged = true;
        }
    }

    void decrement(float dec) {
        if (_externalVar > _min) {
            _externalVar -= dec;
            if (_externalVar < _min) {
                _externalVar = _min;  // Clamp to min
            }
            _hasChanged = true;
        }
    }

    void set(float newVal) {
      if (newVal > _min && newVal < _max) {
          _externalVar = newVal;
          _hasChanged = true;
      }
    }

    // Return the current external variable
    float getStateVariable() const {
        return _externalVar;
    }

    // Check if the state variable has changed
    bool hasChanged() const {
        return _hasChanged;
    }

    // Reset the change flag
    void resetChanged() {
        _hasChanged = false;
    }

    // Override the onEnter method if needed
    void onEnter() override {
        _hasChanged = false;  // Reset the change flag on entering the state
        _externalVar = _init;
    }

private:
    float& _externalVar;  // Reference to the external variable
    float _init;          // Stores the original value of the variable
    float _min;           // Minimum value for the state variable
    float _max;           // Maximum value for the state variable
    bool _hasChanged;     // Flag to track if the state variable has changed
};

class StateWithInt : public State {
/**
 * This class represents a state with an int variable that can be incremented 
 * and decremented in a fixed range.  
 */

public:
    StateWithInt(uint8_t id, FiniteStateAutomaton* automaton, int& externalVar, int minVal, int maxVal)
        : State(id, automaton), _externalVar(externalVar), _init(externalVar), _min(minVal), _max(maxVal), _hasChanged(false) {
          _externalVar = min(_externalVar, _max);
          _externalVar = max(_externalVar, _min);
        }

    void increment(int inc) {
        if (_externalVar < _max) {
            _externalVar += inc;
            if (_externalVar > _max) {
                _externalVar = _max;  // Clamp to max
            }
            _hasChanged = true;
        }
    }

    void decrement(int dec) {
        if (_externalVar > _min) {
            _externalVar -= dec;
            if (_externalVar < _min) {
                _externalVar = _min;  // Clamp to min
            }
            _hasChanged = true;
        }
    }

    void set(int newVal) {
      if (newVal >= _min && newVal <= _max) {
          _externalVar = newVal;
          _hasChanged = true;
      }
    }

    // Return the current external variable
    int getStateVariable() const {
        return _externalVar;
    }

    // Check if the state variable has changed
    bool hasChanged() const {
        return _hasChanged;
    }

    // Reset the change flag
    void resetChanged() {
        _hasChanged = false;
    }

    // Override the onEnter method if needed
    void onEnter() override {
        _hasChanged = false;  // Reset the change flag on entering the state
        _externalVar = _init;
    }

private:
    int& _externalVar;  // Reference to the external variable
    int _init;          // Stores the original value of the variable
    int _min;           // Minimum value for the state variable
    int _max;           // Maximum value for the state variable
    bool _hasChanged;     // Flag to track if the state variable has changed
};

class StateWithBool : public State {
/**
 * This class represents a state with a boolean variable that can be toggled. 
 */

public:
    StateWithBool(uint8_t id, FiniteStateAutomaton* automaton, bool& externalVar)
        : State(id, automaton), _externalVar(externalVar), _init(externalVar), _hasChanged(false) {}

    void toggle() {
        _externalVar = !_externalVar;
        _hasChanged = true;
    }

    // Return the current external variable
    int getStateVariable() const {
        return _externalVar;
    }

    // Check if the state variable has changed
    bool hasChanged() const {
        return _hasChanged;
    }

    // Reset the change flag
    void resetChanged() {
        _hasChanged = false;
    }

    // Override the onEnter method if needed
    void onEnter() override {
        _hasChanged = false;  // Reset the change flag on entering the state
        _externalVar = _init;
    }

private:
    bool& _externalVar;  // Reference to the external variable
    bool _init;          // Stores the original value of the variable
    bool _hasChanged;    // Flag to track if the state variable has changed
};

/* ---------------------------- Utility functions --------------------------- */

String floatToString(float value, int width=6, int decimals=2) {
    char buffer[20]; // Buffer to hold the string representation
    dtostrf(value, width, decimals, buffer); // Convert float to string
    return String(buffer); // Return as String object
}

void updateLCD(const String& firstRow, const String& secondRow) {
  /**
  * Updates the display. It only has two rows (2x16).
  */

  // Clear the screen
  lcd.clear();

  // Print the first row
  lcd.setCursor(0,0);
  lcd.print(firstRow);

  // Print the second row
  lcd.setCursor(0,1);
  lcd.print(secondRow);

}

void setupLCD() {
  /**
  * Setup the LCD.
  */

  // LCD setup
  lcd.init();
  lcd.backlight();
}

String createProgressBar(int percentage) {

  // Ensure the percentage is within 0-100
  if (percentage < 0) percentage = 0;
  if (percentage > 100) percentage = 100;

  // Calculate the number of filled and empty segments
  int filledLength = (percentage / 100.0) * 16; // 16 characters on the LCD
  String progressBar = "";

  // Add filled segments
  for (int i = 0; i < filledLength; i++) {
    progressBar += "#"; // Use '#' to represent filled
  }

  // Add empty segments
  for (int i = filledLength; i < 16; i++) {
    progressBar += " "; // Use space for empty
  }

  return progressBar;
}

/* --------------------------------- States --------------------------------- */

class StateMenuSplashScreen : public State {
public:
    StateMenuSplashScreen(FiniteStateAutomaton* automaton) : State(STATE_MENU_SPLASH_SCREEN, automaton) {}
    void onEnter() override {
        updateLCD("Coil Winder", "     v1.0.0");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_TIMEOUT)
            return automaton->changeState(STATE_WIND);
        return this;
    }
};

class StateWind : public State {
public:
    StateWind(FiniteStateAutomaton* automaton) : State(STATE_WIND, automaton) {}
    void onEnter() override {
        updateLCD("Wind", "");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_UP_PRESS)
            return automaton->changeState(STATE_UNWIND);
        if (event == EVENT_DOWN_PRESS)
            return automaton->changeState(STATE_UNWIND);
        if (event == EVENT_SELECT_PRESS)
            return automaton->changeState(STATE_SET_WIRE_DIAMETER);
        return this;
    }
};

class StateSetWireDiameter : public StateWithFloat {
public:
    StateSetWireDiameter(FiniteStateAutomaton* automaton, float& externalVar) : 
        StateWithFloat(STATE_SET_WIRE_DIAMETER, automaton, externalVar, MIN_WIRE_DIAMETER, MAX_WIRE_DIAMETER) {
        }
    void onEnter() override {
        StateWithFloat::onEnter();
        updateLCD("Wire diameter:", floatToString(getStateVariable()) + " mm");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_SELECT_PRESS)
            return automaton->changeState(STATE_SET_SPOOL_LENGTH);
        if (event == EVENT_UP_PRESS) {
            increment(DELTA_WIRE_DIAMETER);
        }
        if (event == EVENT_DOWN_PRESS) {
            decrement(DELTA_WIRE_DIAMETER);
        }
        if (event == EVENT_UP_LONGPRESS) {
            increment(BIG_DELTA_WIRE_DIAMETER);
        }
        if (event == EVENT_DOWN_LONGPRESS) {
            decrement(BIG_DELTA_WIRE_DIAMETER);
        }

        if (hasChanged()) {
            updateLCD("Wire diameter:", floatToString(getStateVariable()) + " mm");
            resetChanged();
        }

        return this;
    }
};

class StateSetSpoolLength : public StateWithFloat {
public:
    StateSetSpoolLength(FiniteStateAutomaton* automaton, float& externalVar) : 
        StateWithFloat(STATE_SET_SPOOL_LENGTH, automaton, externalVar, MIN_SPOOL_LENGTH, MAX_SPOOL_LENGTH) {}
    void onEnter() override {
        StateWithFloat::onEnter();
        updateLCD("Spool length:", floatToString(getStateVariable()) + " mm");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_SELECT_PRESS)
            return automaton->changeState(STATE_SET_SPOOL_DIAMETER);
        if (event == EVENT_UP_PRESS) {
            increment(DELTA_SPOOL_LENGTH);
        }
        if (event == EVENT_DOWN_PRESS) {
            decrement(DELTA_SPOOL_LENGTH);
        }
        if (event == EVENT_UP_LONGPRESS) {
            increment(BIG_DELTA_SPOOL_LENGTH);
        }
        if (event == EVENT_DOWN_LONGPRESS) {
            decrement(BIG_DELTA_SPOOL_LENGTH);
        }

        if (hasChanged()) {
            updateLCD("Spool length:", floatToString(getStateVariable()) + " mm");
            resetChanged();
        }

        return this;
    }
};


class StateSetSpoolDiameter : public StateWithFloat {
public:
    StateSetSpoolDiameter(FiniteStateAutomaton* automaton, float& externalVar) : 
        StateWithFloat(STATE_SET_SPOOL_DIAMETER, automaton, externalVar, MIN_SPOOL_DIAMETER, MAX_SPOOL_DIAMETER) {}
    void onEnter() override {
        StateWithFloat::onEnter();
        updateLCD("Spool diameter:", floatToString(getStateVariable()) + " mm");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_SELECT_PRESS)
            return automaton->changeState(STATE_SET_LAYER_COUNT);
        if (event == EVENT_UP_PRESS) {
            increment(DELTA_SPOOL_DIAMETER);
        }
        if (event == EVENT_DOWN_PRESS) {
            decrement(DELTA_SPOOL_DIAMETER);
        }
        if (event == EVENT_UP_LONGPRESS) {
            increment(BIG_DELTA_SPOOL_DIAMETER);
        }
        if (event == EVENT_DOWN_LONGPRESS) {
            decrement(BIG_DELTA_SPOOL_DIAMETER);
        }

        if (hasChanged()) {
            updateLCD("Spool diameter:", floatToString(getStateVariable()) + " mm");
            resetChanged();
        }

        return this;
    }
};

class StateSetLayerCount : public StateWithFloat {
public:
    StateSetLayerCount(FiniteStateAutomaton* automaton, float& externalVar) : 
        StateWithFloat(STATE_SET_LAYER_COUNT, automaton, externalVar, MIN_LAYER_COUNT, MAX_LAYER_COUNT) {}
    void onEnter() override {
        StateWithFloat::onEnter();
        updateLCD("Layer count:", floatToString(getStateVariable()));
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_SELECT_PRESS)
            return automaton->changeState(STATE_WIND_ASK_CONFIRM);
        if (event == EVENT_UP_PRESS) {
            increment(DELTA_LAYER_COUNT);
        }
        if (event == EVENT_DOWN_PRESS) {
            decrement(DELTA_LAYER_COUNT);
        }
        if (event == EVENT_UP_LONGPRESS) {
            increment(BIG_DELTA_LAYER_COUNT);
        }
        if (event == EVENT_DOWN_LONGPRESS) {
            decrement(BIG_DELTA_LAYER_COUNT);
        }

        if (hasChanged()) {
            updateLCD("Layer count:", floatToString(getStateVariable()) + " mm");
            resetChanged();
        }

        return this;
    }
};

class StateWindAskConfirm : public State {
public:
    StateWindAskConfirm(FiniteStateAutomaton* automaton) : State(STATE_WIND_ASK_CONFIRM, automaton) {}
    void onEnter() override {
        updateLCD("Start winding?", "");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_SELECT_PRESS)
            return automaton->changeState(STATE_START_WINDING);
        if (event == EVENT_SELECT_LONGPRESS)
            return automaton->changeState(STATE_WIND);
        return this;
    }
};

class StateStartWinding : public StateWithInt {
public:
    StateStartWinding(FiniteStateAutomaton* automaton, int& externalVar) : 
        StateWithInt(STATE_START_WINDING, automaton, externalVar, 0, 2), _progress(0) {}
    void onEnter() override {
        StateWithInt::onEnter();

        // Update the LCD
        updateLCD("Winding...", "");

        // Reset the progress variable
        _progress = 0;

        // Set to 1 to signal we can start the procedure to the outside code
        set(1);
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_RESET) {
            return automaton->changeState(STATE_SET_WIRE_DIAMETER);
        }
        if (event == EVENT_UPDATE_PROGRESS) {
            _progress += 1;
            updateLCD("Winding...", createProgressBar(_progress));
        }
        return this;
    }
private:
    int _progress;
};

class StateUnwind : public State {
public:
    StateUnwind(FiniteStateAutomaton* automaton) : State(STATE_UNWIND, automaton) {}
    void onEnter() override {
        updateLCD("Unwind", "");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_UP_PRESS)
            return automaton->changeState(STATE_WIND);
        if (event == EVENT_DOWN_PRESS)
            return automaton->changeState(STATE_WIND);
        if (event == EVENT_SELECT_PRESS)
            return automaton->changeState(STATE_SET_TIME);
        return this;
    }
};

class StateSetTime : public StateWithFloat {
public:
    StateSetTime(FiniteStateAutomaton* automaton, float& externalVar) :
        StateWithFloat(STATE_SET_TIME, automaton, externalVar, MIN_TIME, MAX_TIME) {}
    void onEnter() override {
        StateWithFloat::onEnter();
        updateLCD("Time:", floatToString(getStateVariable()) + " s");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_SELECT_PRESS)
            return automaton->changeState(STATE_SET_SPEED);
        if (event == EVENT_UP_PRESS) {
            increment(DELTA_TIME);
        }
        if (event == EVENT_DOWN_PRESS) {
            decrement(DELTA_TIME);
        }
        if (event == EVENT_UP_LONGPRESS) {
            increment(BIG_DELTA_TIME);
        }
        if (event == EVENT_DOWN_LONGPRESS) {
            decrement(BIG_DELTA_TIME);
        }

        if (hasChanged()) {
            updateLCD("Time:", floatToString(getStateVariable()) + " s");
            resetChanged();
        }

        return this;
    }
};

class StateSetSpeed : public StateWithFloat {
public:
    StateSetSpeed(FiniteStateAutomaton* automaton, float& externalVar) :
        StateWithFloat(STATE_SET_SPEED, automaton, externalVar, MIN_VELOCITY_STEPS_S, MAX_VELOCITY_STEPS_S) {}
    void onEnter() override {
        StateWithFloat::onEnter();
        updateLCD("Speed:", floatToString(getStateVariable()) + " steps/s");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_SELECT_PRESS)
            return automaton->changeState(STATE_SET_DIRECTION);
        if (event == EVENT_UP_PRESS) {
            increment(DELTA_SPEED);
        }
        if (event == EVENT_DOWN_PRESS) {
            decrement(DELTA_SPEED);
        }
        if (event == EVENT_UP_LONGPRESS) {
            increment(BIG_DELTA_SPEED);
        }
        if (event == EVENT_DOWN_LONGPRESS) {
            decrement(BIG_DELTA_SPEED);
        }

        if (hasChanged()) {
            updateLCD("Speed:", floatToString(getStateVariable()) + " steps/s");
            resetChanged();
        }

        return this;
    }
};

class StateSetDirection : public StateWithBool {
public:
    StateSetDirection(FiniteStateAutomaton* automaton, bool& externalVar) :
        StateWithBool(STATE_SET_DIRECTION, automaton, externalVar){}
    void onEnter() override {
        StateWithBool::onEnter();
        updateLCD("Direction:", getStateVariable() ? "Forward" : "Backward");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_UP_PRESS) {
            toggle();
        }
        if (event == EVENT_DOWN_PRESS) {
            toggle();
        }

        if (event == EVENT_SELECT_PRESS) {
            return automaton->changeState(STATE_UNWIND_ASK_CONFIRM);
        }

        if (hasChanged()) {
            updateLCD("Direction:", getStateVariable() ? "Forward" : "Backward");
            resetChanged();
        }

        return this;
    }
};

class StateUnwindAskConfirm : public State {
public:
    StateUnwindAskConfirm(FiniteStateAutomaton* automaton) : State(STATE_UNWIND_ASK_CONFIRM, automaton) {}
    void onEnter() override {
        updateLCD("Start unwinding?", "");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_SELECT_PRESS)
            return automaton->changeState(STATE_START_UNWINDING);
        if (event == EVENT_SELECT_LONGPRESS)
            return automaton->changeState(STATE_UNWIND);
        return this;
    }
};

class StateStartUnwinding : public StateWithInt {
public:
    StateStartUnwinding(FiniteStateAutomaton* automaton, int& externalVar) :
        StateWithInt(STATE_START_UNWINDING, automaton, externalVar, 0, 2), _progress(0) {}
    void onEnter() override {
        StateWithInt::onEnter();

        // Update the LCD
        updateLCD("Unwinding...", "");

        // Reset the progress variable
        _progress = 0;

        // Set to 2 to signal we can start the procedure to the outside code
        set(2);
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_RESET) {
            return automaton->changeState(STATE_SET_SPEED);
        }
        if (event == EVENT_UPDATE_PROGRESS) {
            _progress += 1;
            updateLCD("Unwinding...", createProgressBar(_progress));
        }
        return this;
    }
private:
    int _progress;
};
