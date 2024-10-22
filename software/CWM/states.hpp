#include "automaton.hpp"

// The only I didn't end up writing
#include <LiquidCrystal_I2C.h>


// LCD settings
LiquidCrystal_I2C lcd(0x27, 16, 2);

class StateWithRange : public State {
/**
 * This class represents a state with a variable that can be DELTAed 
 * and decremented in a fixed range.  
 */

public:
    StateWithRange(uint8_t id, FiniteStateAutomaton* automaton, float& externalVar, float minVal, float maxVal)
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
        // Custom behavior when entering this state can be added here
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
        // Custom behavior when entering this state can be added here
        _hasChanged = false;  // Reset the change flag on entering the state
        _externalVar = _init;
    }

private:
    bool& _externalVar;  // Reference to the external variable
    bool _init;          // Stores the original value of the variable
    bool _hasChanged;    // Flag to track if the state variable has changed
};

/* ---------------------------- Utility functions --------------------------- */

String floatToString(float value, int width = 6, int decimals = 2) {
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
            return automaton->changeState(STATE_SET_WIRE_DIAMETER);
        return this;
    }
};

class StateSetWireDiameter : public StateWithRange {
public:
    StateSetWireDiameter(FiniteStateAutomaton* automaton, float& externalVar) : 
        StateWithRange(STATE_SET_WIRE_DIAMETER, automaton, externalVar, MIN_WIRE_DIAMETER, MAX_WIRE_DIAMETER) {}
    void onEnter() override {
        StateWithRange::onEnter();
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

class StateSetSpoolLength : public StateWithRange {
public:
    StateSetSpoolLength(FiniteStateAutomaton* automaton, float& externalVar) : 
        StateWithRange(STATE_SET_SPOOL_LENGTH, automaton, externalVar, MIN_SPOOL_LENGTH, MAX_SPOOL_LENGTH) {}
    void onEnter() override {
        StateWithRange::onEnter();
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


class StateSetSpoolDiameter : public StateWithRange {
public:
    StateSetSpoolDiameter(FiniteStateAutomaton* automaton, float& externalVar) : 
        StateWithRange(STATE_SET_SPOOL_DIAMETER, automaton, externalVar, MIN_SPOOL_DIAMETER, MAX_SPOOL_DIAMETER) {}
    void onEnter() override {
        StateWithRange::onEnter();
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

class StateSetLayerCount : public StateWithRange {
public:
    StateSetLayerCount(FiniteStateAutomaton* automaton, float& externalVar) : 
        StateWithRange(STATE_SET_LAYER_COUNT, automaton, externalVar, MIN_LAYER_COUNT, MAX_LAYER_COUNT) {}
    void onEnter() override {
        StateWithRange::onEnter();
        updateLCD("Layer count:", floatToString(getStateVariable()) + " mm");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_SELECT_PRESS)
            return automaton->changeState(STATE_ASK_CONFIRM);
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

class StateAskConfirm : public State {
public:
    StateAskConfirm(FiniteStateAutomaton* automaton) : State(STATE_ASK_CONFIRM, automaton) {}
    void onEnter() override {
        updateLCD("Start winding?", "");
    }
    State* onEvent(const uint8_t& event) override {
        if (event == EVENT_SELECT_PRESS)
            return automaton->changeState(STATE_START_WINDING);
        if (event == EVENT_SELECT_LONGPRESS)
            return automaton->changeState(STATE_SET_WIRE_DIAMETER);
        return this;
    }
};

class StateStartWinding : public StateWithBool {
public:
    StateStartWinding(FiniteStateAutomaton* automaton, bool& externalVar) : 
        StateWithBool(STATE_START_WINDING, automaton, externalVar), _progress(0) {}
    void onEnter() override {
        StateWithBool::onEnter();

        // Update the LCD
        updateLCD("Winding...", "");

        // Reset the progress variable
        _progress = 0;

        // Toggle to signal we can start the procedure to the outside code
        toggle();
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
