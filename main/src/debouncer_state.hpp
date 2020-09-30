#ifndef __DEBOUNCER_STATE_H
#define __DEBOUNCER_STATE_H

#include <cstdint>

enum PinState {
        ON = false,
        OFF = true
};

class DebouncerState {
    public:
        DebouncerState(uint16_t treshhold);
        ~DebouncerState();
        void updateState(int32_t gpio_level);
        PinState getPinState();
    private:
        PinState m_pin_state;
        uint8_t m_signal_count;
        uint16_t m_threshhold;
};

#endif