#ifndef __DEBOUNCER_H
#define __DEBOUNCER_H

#include "driver/gpio.h"

enum PinState {
        ON = false,
        OFF = true
};

struct GpioState {
    gpio_num_t pin;
    PinState pin_state;
    uint8_t signal_count;
    uint16_t threshhold;
    uint64_t check_interval;
};

class Debouncer {
    public:
        Debouncer(uint16_t threshhold, uint64_t check_interval);
        ~Debouncer();
        void registerGpioPin(gpio_num_t pin);
        PinState getPinState();
};

#endif