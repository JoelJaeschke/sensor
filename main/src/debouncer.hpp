#ifndef __DEBOUNCER_H
#define __DEBOUNCER_H

#include "driver/gpio.h"
#include "esp_log.h"

#include "debouncer_state.hpp"
#include "wrapper.hpp"

template<gpio_num_t Pin, uint32_t Check_Interval, uint8_t Threshhold>
class Debouncer {
    public:
        Debouncer():    m_gpio(Gpio<Pin, true>()),
                        m_timer(Timer<Check_Interval>("debounceTimer")),
                        m_state(DebouncerState(Threshhold))
        {
            ESP_LOGD("Debouncer", "Debouncer ctor called");
            
            // Check if gpio can run, otherwise abort
            ESP_ERROR_CHECK(m_gpio.isActive());
            
            // Register callback, if it fails, we need to abort
            ESP_ERROR_CHECK(m_timer.registerCallback(debounce_callback, static_cast<void*>(this)));
            ESP_LOGD("Debouncer", "Registered callback");

            // start the timer, if it fails, we again, need to abort
            ESP_ERROR_CHECK(m_timer.startTimerPeriodic());
            ESP_LOGD("Debouncer", "Started timer");
        };

        Debouncer(Gpio<Pin> pin):   m_gpio(pin),
                                    m_timer(Timer<Check_Interval>("debounceTimer")),
                                    m_state(DebouncerState(Threshhold))
        {
            ESP_LOGD("Debouncer", "Debouncer ctor called");
            
            // Check if gpio can run, otherwise abort
            ESP_ERROR_CHECK(m_gpio.isActive());

            // Register callback
            ESP_ERROR_CHECK(m_timer.registerCallback(debounce_callback, static_cast<void*>(this)));
            ESP_LOGD("Debouncer", "Registered callback");
            
            // start the timer
            ESP_ERROR_CHECK(m_timer.startTimerPeriodic());
            ESP_LOGD("Debouncer", "Started timer");
        };

        ~Debouncer() {
            ESP_LOGD("Debouncer", "Debouncer dtor called");
        };

        PinState level() {
            return m_state.getPinState();
        };
    private:
        // Use a private static method as callback
        // A local static function would not have access to member objects of class
        static void debounce_callback(void* context) {
            Debouncer* dbcer = reinterpret_cast<Debouncer*>(context);
            
            auto level = dbcer->m_gpio.level();
            dbcer->m_state.updateState(level);

            ESP_LOGV("Debouncer", "Callback called");
        };

        Gpio<Pin, true> m_gpio;
        Timer<Check_Interval> m_timer;
        DebouncerState m_state;
};

#endif