#ifndef __SENSOR_READER_H
#define __SENSOR_READER_H

#include <stdint.h>
#include "freertos/queue.h"
#include "driver/gpio.h"

#define RECEIVER_PIN GPIO_NUM_23
#define LED_CHANNEL 0
#define CHANNEL_RESOLUTION 8
#define DIODE_FREQUENCY 38000
#define PASSING_THRESHOLD 250
#define IR_DIODE_PIN GPIO_NUM_18
#define NUM_ACTIVE 4
#define CHECK_INTERVAL 5000

class SensorReader {
    public:
        SensorReader();
        ~SensorReader();
        void setup();
        void process();
    private:
        void startPass();
        void endPass();

        uint64_t m_num_passes;
        uint64_t m_start_time;
        uint64_t m_end_time;
        bool m_previously_on;
};

#endif