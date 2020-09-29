#include "esp_log.h"
#include "driver/timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wrapper.hpp"
#include "debouncer.hpp"

static const char* TAG = "Debouncer";

Debouncer::Debouncer(uint16_t threshhold): m_threshhold(threshhold)
{
    ESP_LOGV(TAG, "Debouncer ctor called");
}

Debouncer::~Debouncer() {
    ESP_LOGV(TAG, "Debouncer dtor called");
}

// gpio_level == 1 means high/active, gpio_level == 0 means low/inactive
void Debouncer::updateState(int32_t gpio_level) {
    if (gpio_level == 1) {
        m_signal_count++;
    } else {
        m_signal_count--;
    }

    if (m_signal_count > m_threshhold) m_signal_count = m_threshhold;

    if (m_signal_count == m_threshhold) {
        m_pin_state = PinState::ON;
    } else {
        m_pin_state = PinState::OFF;
    }
}

PinState Debouncer::getPinState() {
    ESP_LOGV(TAG, "Getting debouncer pin state: %s", m_pin_state == PinState::ON ? "on" : "off");
    return m_pin_state;
}