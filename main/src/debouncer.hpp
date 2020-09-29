#ifndef __DEBOUNCER_H
#define __DEBOUNCER_H

#include "driver/gpio.h"

enum PinState {
        ON = false,
        OFF = true
};

class Debouncer {
    public:
        Debouncer(uint16_t treshhold);
        ~Debouncer();
        void updateState(int32_t gpio_level);
        PinState getPinState();
    private:
        PinState m_pin_state;
        uint8_t m_signal_count;
        uint16_t m_threshhold;
};

#endif