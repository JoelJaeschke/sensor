#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/timer.h"

#include "pwm_driver.h"

static const char* TAG = "PwmDriver";

PwmDriver::PwmDriver(uint32_t frequency, gpio_num_t pin) {
    ESP_LOGV(TAG, "PwmDriver ctor called");

    ESP_LOGV(TAG, "Initializing ledc timer config");
    // Setup PWM signal for diode (see: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html#ledc-api-configure-channel)
    // Configure timer 
    ledc_timer_config_t diode_timer_conf;
    diode_timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    diode_timer_conf.duty_resolution = LEDC_TIMER_8_BIT;
    diode_timer_conf.timer_num = LEDC_TIMER_0;
    diode_timer_conf.freq_hz = frequency;
    diode_timer_conf.clk_cfg = LEDC_AUTO_CLK;

    ESP_ERROR_CHECK(ledc_timer_config(&diode_timer_conf));

    ESP_LOGV(TAG, "Initializing ledc channel config");
    // Configure channel
    ledc_channel_config_t channel_conf;
    channel_conf.gpio_num = pin;
    channel_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    channel_conf.channel = LEDC_CHANNEL_0;
    channel_conf.intr_type = LEDC_INTR_DISABLE;
    channel_conf.timer_sel = LEDC_TIMER_0;
    channel_conf.duty = 200;
    channel_conf.hpoint = 0;

    ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));
};

PwmDriver::~PwmDriver() {
    ESP_LOGV(TAG, "PwmDriver dtor called");
};