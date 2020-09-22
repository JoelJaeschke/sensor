#ifndef __DIODE_PWM_DRIVER_H
#define __DIODE_PWM_DRIVER_H

#include "driver/gpio.h"

class PwmDriver {
    public:
        PwmDriver(uint32_t frequency, gpio_num_t pin);
        ~PwmDriver();
};

#endif