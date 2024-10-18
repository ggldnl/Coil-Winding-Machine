#ifndef AUTOMATON_HPP
#define AUTOMATON_HPP

#include <Arduino.h>

class FiniteStateAutomaton; // Forward declaration

class State {
public:
    uint8_t id;
    FiniteStateAutomaton* automaton;  // Pointer to the automaton

    State(uint8_t id, FiniteStateAutomaton* automaton);
    virtual ~State();

    // Method called when entering the state
    virtual void onEnter();

    // Method to handle events
    virtual State* onEvent(const uint8_t& event);
};

// Finite State Automaton class
class FiniteStateAutomaton {
public:
    FiniteStateAutomaton() : currentState(0), stateCount(0) {}

    ~FiniteStateAutomaton() {
        for (int i = 0; i < stateCount; i++) {
            delete states[i];
        }
    }

    // Add a state to the automaton
    void addState(State* state) {
        if (stateCount < maxStates) { // Ensure we don't exceed the array size
            states[stateCount++] = state;
        } 
        /*
        else {
            // Handle the case when too many states are added
            // Serial.println("Error: Maximum number of states reached: " + String(stateCount) + "/" + String(maxStates));
        }
        */
    }

    // Start the automaton on the provided state
    void start(const uint8_t& stateID) {
        State* state = findStateByID(stateID);
        if (state != nullptr) {
            currentState = state;
            currentState->onEnter();
        } 
        /* else {
            Serial.println("Error: State '" + String(stateID) + "' does not exist.");
        }
        */
    }

    // Change the current state
    State* changeState(const uint8_t& stateID) {
        State* state = findStateByID(stateID);
        if (state != nullptr) {
            currentState = state;
            currentState->onEnter();
        } 
        /*
        else {
            Serial.println("Error: State '" + String(stateID) + "' does not exist.");
        }
        */
        return state;
    }

    // Handle events
    void onEvent(const uint8_t& event) {
        if (currentState == nullptr) {
            // Serial.println("Error: Automaton has not been initialized yet.");
            return;
        }
        State* newState = currentState->onEvent(event);
        if (newState != nullptr && newState != currentState) {
            currentState = newState;
            currentState->onEnter();
        }
    }

private:

    State* states[20]; // Array to store up to 20 states
    uint8_t maxStates = 20; // Maximum number of states we can add
    uint8_t stateCount = 0; // Number of added states
    State* currentState;

    // Helper function to find a state by ID
    State* findStateByID(const uint8_t& stateID) {
        for (int i = 0; i < stateCount; i++) {
            if (states[i]->id == stateID) {
                return states[i];
            }
        }
        return nullptr;
    }
};

// Implementation of State methods
State::State(uint8_t id, FiniteStateAutomaton* automaton) {
    this->id = id;
    this->automaton = automaton;
}

State::~State() {}

void State::onEnter(void) {}

State* State::onEvent(const uint8_t& event) {
    return this;
}

#endif  // AUTOMATON_HPP
