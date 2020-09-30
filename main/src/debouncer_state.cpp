#include "esp_log.h"

#include "wrapper.hpp"
#include "debouncer_state.hpp"

DebouncerState::DebouncerState(uint16_t threshhold): m_threshhold(threshhold)
{
    ESP_LOGD("DebouncerState", "Debouncer ctor called");
}

DebouncerState::~DebouncerState() {
    ESP_LOGD("DebouncerState", "Debouncer dtor called");
}

// gpio_level == 1 means high/active, gpio_level == 0 means low/inactive
void DebouncerState::updateState(int32_t gpio_level) {
    ESP_LOGV("DebouncerState", "State change: %s", gpio_level == 1 ? "+1" : "-1");
    if (gpio_level == 1) {
        m_signal_count++;
    } else {
        m_signal_count--;
    }


    if (m_signal_count > m_threshhold) {
        ESP_LOGV("DebouncerState", "Threshhold reached");
        m_signal_count = m_threshhold;
        m_pin_state = PinState::ON;
    } else {
        ESP_LOGV("DebouncerState", "Threshhold reached");
        m_pin_state = PinState::OFF;
    }
}

PinState DebouncerState::getPinState() {
    ESP_LOGV("DebouncerState", "Getting debouncer pin state: %s", m_pin_state == PinState::ON ? "on" : "off");
    return m_pin_state;
}