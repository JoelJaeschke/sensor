#ifndef __SENSOR_READER_H
#define __SENSOR_READER_H

#include <cstdint>
#include <optional>
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "debouncer.h"
#include "pwm_driver.h"
#include "ipc_struct.h"

#define RECEIVER_PIN GPIO_NUM_23
#define LED_CHANNEL 0
#define CHANNEL_RESOLUTION 8
#define DIODE_FREQUENCY 38000
#define PASSING_THRESHOLD 250
#define IR_DIODE_PIN GPIO_NUM_18
#define NUM_ACTIVE 4
#define CHECK_INTERVAL 5000

typedef struct CurrentPass {
    uint64_t start_time;
    uint64_t end_time;
    bool previously_on;
} CurrentPass;

class SensorReader {
    public:
        SensorReader();
        ~SensorReader();
        std::optional<Pass> process();
        uint64_t getNumberOfPasses();
    private:
        void startPass();
        std::optional<Pass> endPass();

        uint64_t m_num_passes;
        CurrentPass m_current_pass;
        Debouncer m_receiver_debouncer;
        PwmDriver m_diode_driver;
};

#endif