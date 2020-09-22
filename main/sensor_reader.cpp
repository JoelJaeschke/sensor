#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sensor_reader.h"

// --- Private implementation --- //
namespace {
    // Inverted, since on is signalled by logical low
    enum PinState {
        ON = false,
        OFF = true
    };

    struct GpioState {
        uint8_t signal_count;
        PinState pin_state;
    };

    struct GpioState global_gpio_state {0, PinState::OFF};

    void debounce_isr(void* arg) {
        int lvl = !gpio_get_level(RECEIVER_PIN);

        // Add one to signal count, if lvl is 0
        global_gpio_state.signal_count += lvl*1;
        
        // Subtract one from signal count if lvl is 1
        global_gpio_state.signal_count -= (!lvl)*1;
        
        // Limit signal count to max NUM_ACTIVE
        global_gpio_state.signal_count = global_gpio_state.signal_count > NUM_ACTIVE ? NUM_ACTIVE : global_gpio_state.signal_count;

        // If signal_count >= NUM_ACTIVE, activate pin
        // Else deactivate pin
        // TODO: Replace if with some bit shift magic
        if (global_gpio_state.signal_count == NUM_ACTIVE) {
            global_gpio_state.pin_state = PinState::ON;
        } else {
            global_gpio_state.pin_state = PinState::OFF;
        };
    };
}

// --- Public implementation --- //
SensorReader::SensorReader():   m_num_passes(0),
                                m_start_time(0),
                                m_end_time(0),
                                m_previously_on(false) {};

SensorReader::~SensorReader() {};

void SensorReader::setup() {
    // Setup receiver pin 
    gpio_reset_pin(RECEIVER_PIN);
    gpio_set_direction(RECEIVER_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RECEIVER_PIN, GPIO_PULLUP_ONLY);
    gpio_intr_disable(RECEIVER_PIN);

    // Setup PWM signal for diode (see: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html#ledc-api-configure-channel)
    // Configure timer 
    ledc_timer_config_t diode_timer_conf;
    diode_timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    diode_timer_conf.duty_resolution = LEDC_TIMER_8_BIT;
    diode_timer_conf.timer_num = LEDC_TIMER_0;
    diode_timer_conf.freq_hz = DIODE_FREQUENCY;
    diode_timer_conf.clk_cfg = LEDC_AUTO_CLK;

    ledc_timer_config(&diode_timer_conf);

    // Configure channel
    ledc_channel_config_t channel_conf;
    channel_conf.gpio_num = IR_DIODE_PIN;
    channel_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    channel_conf.channel = LEDC_CHANNEL_0;
    channel_conf.intr_type = LEDC_INTR_DISABLE;
    channel_conf.timer_sel = LEDC_TIMER_0;
    channel_conf.duty = 200;

    ledc_channel_config(&channel_conf);

    // Setup timer to read receiver state every 5 milliseconds
    // Update internal counter to reflect number of positive reads
    esp_timer_handle_t timer_handle;
    esp_timer_create_args_t timer_conf;
    timer_conf.name = "debounceTimer";
    timer_conf.arg = NULL;
    timer_conf.callback = debounce_isr;
    timer_conf.dispatch_method = ESP_TIMER_TASK;

    esp_timer_create(&timer_conf, &timer_handle);

    esp_timer_start_periodic(timer_handle, CHECK_INTERVAL);

    return;
}

void SensorReader::process() {
    // If sensor was previously off and is now turned on, we set previously on and
    // start a timer
    if (m_previously_on == false && global_gpio_state.pin_state == PinState::ON) {
        startPass();
    }

    // If sensor was previously on and is now turned off, we set previously on to false
    // and stop the timer
    if (m_previously_on == true && global_gpio_state.pin_state == PinState::OFF) {
        endPass();
    }

    return;
}

void SensorReader::startPass() {
    m_previously_on = true;
    m_start_time = esp_timer_get_time();
}

void SensorReader::endPass() {
    m_end_time = esp_timer_get_time();
    uint64_t duration = (m_end_time - m_start_time) / 1000;
    m_num_passes++;

    m_previously_on = false;
    m_end_time = 0;
    m_start_time = 0;

    if (duration >= PASSING_THRESHOLD) {
        printf("Pass took %lld milliseconds.\n", duration);
    }
}