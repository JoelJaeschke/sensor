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
        Debouncer(uint16_t threshhold, uint64_t check_interval, gpio_num_t pin);
        ~Debouncer();
        PinState getPinState();
    private:
        void registerGpioPin();

};

#endif