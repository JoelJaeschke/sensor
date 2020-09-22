#include "driver/timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "debouncer.h"

namespace {
    struct GpioState debouncer_gpio_state {GPIO_NUM_0, PinState::OFF, 0, 0, 0};

    void debounce_routine(void* arg) {
        int lvl = !gpio_get_level(debouncer_gpio_state.pin);

        // Add one to signal count, if lvl is 0
        debouncer_gpio_state.signal_count += lvl*1;
        
        // Subtract one from signal count if lvl is 1
        debouncer_gpio_state.signal_count -= (!lvl)*1;
        
        // Limit signal count to max NUM_ACTIVE
        debouncer_gpio_state.signal_count = 
            debouncer_gpio_state.signal_count > debouncer_gpio_state.threshhold ? 
            debouncer_gpio_state.threshhold : debouncer_gpio_state.signal_count;

        // If signal_count >= NUM_ACTIVE, activate pin
        // Else deactivate pin
        // TODO: Replace if with some bit shift magic
        if (debouncer_gpio_state.signal_count == debouncer_gpio_state.threshhold) {
            debouncer_gpio_state.pin_state = PinState::ON;
        } else {
            debouncer_gpio_state.pin_state = PinState::OFF;
        };
    }
}


Debouncer::Debouncer(uint16_t threshhold, uint64_t check_interval, gpio_num_t pin)
{
    // Configure global GpioState Struct
    debouncer_gpio_state.threshhold = threshhold;
    debouncer_gpio_state.check_interval = check_interval;
    debouncer_gpio_state.pin = pin;

    registerGpioPin();
};

void Debouncer::registerGpioPin() {
    // Setup gpio pin
    gpio_reset_pin(debouncer_gpio_state.pin);
    gpio_set_direction(debouncer_gpio_state.pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(debouncer_gpio_state.pin, GPIO_PULLUP_ONLY);
    gpio_intr_disable(debouncer_gpio_state.pin);

    // Setup timer
    esp_timer_handle_t timer_handle;
    esp_timer_create_args_t timer_conf;
    timer_conf.name = "debounceTimer";
    timer_conf.arg = NULL;
    timer_conf.callback = debounce_routine;
    timer_conf.dispatch_method = ESP_TIMER_TASK;

    // create timer
    esp_timer_create(&timer_conf, &timer_handle);

    // start timer
    esp_timer_start_periodic(timer_handle, debouncer_gpio_state.check_interval);
};

PinState Debouncer::getPinState() {
    return debouncer_gpio_state.pin_state;
}