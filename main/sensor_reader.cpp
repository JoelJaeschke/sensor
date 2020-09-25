#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "sensor_reader.h"

static const char* TAG = "SensorReader";

SensorReader::SensorReader():   m_num_passes(0),
                                m_current_pass({0, 0, false}),
                                m_receiver_debouncer(Debouncer(NUM_ACTIVE, CHECK_INTERVAL, RECEIVER_PIN)),
                                m_diode_driver(PwmDriver(DIODE_FREQUENCY, IR_DIODE_PIN)) 
{
    ESP_LOGV(TAG, "SensorReader ctor called");
};

SensorReader::~SensorReader() {
    ESP_LOGV(TAG, "SensorReader dtor called");
};

std::optional<Pass> SensorReader::process() {
    // If sensor was previously off and is now turned on, we set previously on and
    // start a timer
    if (m_current_pass.previously_on == false && m_receiver_debouncer.getPinState() == PinState::ON) {
        startPass();
    }

    // If sensor was previously on and is now turned off, we set previously on to false
    // and stop the timer
    if (m_current_pass.previously_on == true && m_receiver_debouncer.getPinState() == PinState::OFF) {
        return endPass();
    }

    return std::nullopt;
}

uint64_t SensorReader::getNumberOfPasses() {
    return m_num_passes;
}

void SensorReader::startPass() {
    m_current_pass.previously_on = true;
    m_current_pass.start_time = esp_timer_get_time();
    ESP_LOGV(TAG, "Start of pass detected. Time is: %lld", m_current_pass.start_time);
}

std::optional<Pass> SensorReader::endPass() {
    m_current_pass.end_time = esp_timer_get_time();
    uint16_t duration = static_cast<uint16_t>((m_current_pass.end_time - m_current_pass.start_time) / 1000);

    ESP_LOGV(TAG, "End of pass detected. Time is: %lld. Duration is: %d", m_current_pass.end_time, duration);

    if (duration >= PASSING_THRESHOLD) {
        m_num_passes++;
        Pass pass = {m_current_pass.end_time, duration};
        
        ESP_LOGV(TAG, "Pass duration was bigger than threshhold (%dms)", duration);
        
        m_current_pass.previously_on = false;
        m_current_pass.end_time = 0;
        m_current_pass.start_time = 0;

        return std::optional<Pass>{pass};
    }

    m_current_pass.previously_on = false;
    m_current_pass.end_time = 0;
    m_current_pass.start_time = 0;

    return std::nullopt;
}