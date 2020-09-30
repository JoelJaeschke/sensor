#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "sensor_reader.hpp"
#include "wrapper.hpp"

SensorReader::SensorReader():   m_num_passes(0),
                                m_current_pass({0, 0, false}),
                                m_receiver_debouncer(Debouncer<RECEIVER_PIN, CHECK_INTERVAL, PASSING_THRESHOLD>()),
                                m_diode_driver(PwmDriver<IR_DIODE_PIN, DIODE_FREQUENCY>())
{
    ESP_LOGD("SensorReader", "SensorReader ctor called");

    // Check if PWM driver is working
    ESP_ERROR_CHECK(m_diode_driver.isActive());

    // Check for receiver debouncer omitted, it will crash on failure
};

SensorReader::~SensorReader() {
    ESP_LOGD("SensorReader", "SensorReader dtor called");
};

std::optional<Pass> SensorReader::process() {
    // If sensor was previously off and is now turned on, we set previously on and
    // start a timer
    if (m_current_pass.previously_on == false && m_receiver_debouncer.level() == PinState::ON) {
        startPass();
    }

    // If sensor was previously on and is now turned off, we set previously on to false
    // and stop the timer
    if (m_current_pass.previously_on == true && m_receiver_debouncer.level() == PinState::OFF) {
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

    ESP_LOGD("SensorReader", "Start of pass detected. Time is: %lld", m_current_pass.start_time);
}

std::optional<Pass> SensorReader::endPass() {
    m_current_pass.end_time = esp_timer_get_time();
    uint16_t duration = static_cast<uint16_t>((m_current_pass.end_time - m_current_pass.start_time) / 1000);

    ESP_LOGD("SensorReader", "End of pass detected. Time is: %lld. Duration is: %d", m_current_pass.end_time, duration);

    if (duration >= PASSING_THRESHOLD) {
        ESP_LOGD("SensorReader", "Pass duration was bigger than threshhold (%dms)", duration);
        
        m_num_passes++;
        Pass pass = {m_current_pass.end_time, duration};
        
        resetCurrentPass();

        return std::optional<Pass>{pass};
    }

    resetCurrentPass();

    return std::nullopt;
}

void SensorReader::resetCurrentPass() {
    // Reset current pass
    m_current_pass.previously_on = false;
    m_current_pass.end_time = 0;
    m_current_pass.start_time = 0;
}