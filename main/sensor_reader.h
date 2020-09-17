#ifndef __SENSOR_READER_H
#define __SENSOR_READER_H

#include <stdint.h>
#include "driver/gpio.h"

#define LED_CHANNEL 0
#define RECEIVER_PIN GPIO_NUM_23
#define IR_DIODE_PIN GPIO_NUM_18
#define CHANNEL_RESOLUTION 8
#define DIODE_FREQUENCY 38000
#define PASSING_THRESHOLD 400

class SensorReader {
    public:
        SensorReader();
        ~SensorReader();
        void setup();
        void process();
        void start_pass();
    private:
        uint64_t m_num_passes;
        int64_t m_start_pass;
        int64_t m_end_pass;
};

#endif