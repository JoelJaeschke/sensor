#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sensor_reader.h"
#include "debouncer.h"

// --- Public implementation --- //
SensorReader::SensorReader():   m_num_passes(0),
                                m_current_pass({0, 0, false}),
                                m_receiver_debouncer(Debouncer(NUM_ACTIVE, CHECK_INTERVAL)) {};

SensorReader::~SensorReader() {};

void SensorReader::setup() {
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

    m_receiver_debouncer.registerGpioPin(RECEIVER_PIN);

    return;
}

void SensorReader::process() {
    // If sensor was previously off and is now turned on, we set previously on and
    // start a timer
    if (m_current_pass.previously_on == false && m_receiver_debouncer.getPinState() == PinState::ON) {
        startPass();
    }

    // If sensor was previously on and is now turned off, we set previously on to false
    // and stop the timer
    if (m_current_pass.previously_on == true && m_receiver_debouncer.getPinState() == PinState::OFF) {
        endPass();
    }

    return;
}

uint64_t SensorReader::getNumberOfPasses() {
    return m_num_passes;
}

void SensorReader::startPass() {
    m_current_pass.previously_on = true;
    m_current_pass.start_time = esp_timer_get_time();
}

void SensorReader::endPass() {
    m_current_pass.end_time = esp_timer_get_time();
    uint64_t duration = (m_current_pass.end_time - m_current_pass.start_time) / 1000;
    m_num_passes++;

    m_current_pass.previously_on = false;
    m_current_pass.end_time = 0;
    m_current_pass.start_time = 0;

    if (duration >= PASSING_THRESHOLD) {
        printf("Pass took %lld milliseconds.\n", duration);
    }
}