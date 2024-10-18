#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

#define PRESSED LOW
#define RELEASED HIGH

class Button {
public:
    Button(uint8_t pin, uint16_t debounce_ms = 50)
        : _pin(pin), _delay(debounce_ms), _state(RELEASED),
          _ignore_until(0), _pressed_time(0), _has_changed(false) {
        pinMode(_pin, INPUT_PULLUP);
    }

    uint8_t getPin() {
        return _pin; 
    }

    bool read() {
        if (_ignore_until > millis()) {
            // Ignore any changes until debounce period has passed
        }
        else if (digitalRead(_pin) != _state) {
            _ignore_until = millis() + _delay;
            _state = !_state;
            _has_changed = true;

            if (_state == PRESSED) {
                _pressed_time = millis();  // Start timing when button is pressed
            } else {
                _pressed_time = 0;  // Reset pressed time when button is released
            }
        }

        return _state;
    }

    bool hasChanged() {
        if (_has_changed) {
            _has_changed = false;
            return true;
        }
        return false;
    }

    bool pressed() {
        return (read() == PRESSED && hasChanged());
    }

    bool released() {
        return (read() == RELEASED && hasChanged());
    }

    bool isHeld(uint16_t hold_time_ms = 800) {
        if (_state == PRESSED && (millis() - _pressed_time >= hold_time_ms)) {
            return true;
        }
        return false;
    }

private:
    uint8_t _pin;
    uint16_t _delay;
    uint8_t _state;
    unsigned long _ignore_until;
    unsigned long _pressed_time;
    bool _has_changed;
};

#endif // BUTTON_H
