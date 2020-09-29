#ifndef __WRAPPER_H
#define __WRAPPER_H

#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/timer.h"

template <gpio_num_t Pin>
class Gpio {
    public:
        Gpio(): m_active(true),
                m_intr_enable(false),
                m_pin_mode(GPIO_MODE_INPUT),
                m_pull_mode(GPIO_PULLUP_ONLY)
        {
            // Setup a pin with default values as a floating input;
            gpio_reset_pin(Pin);
            esp_err_t direction_err = gpio_set_direction(Pin, m_pin_mode);
            esp_err_t mode_err = gpio_set_pull_mode(Pin, m_pull_mode);
            esp_err_t intr_err = gpio_intr_disable(Pin);

            if (!(direction_err == ESP_OK && mode_err == ESP_OK && intr_err == ESP_OK)) m_active = false;
        };

        Gpio(gpio_mode_t pin_mode, gpio_pull_mode_t pull_mode, bool intr_enable):   m_active(true),
                                                                                    m_intr_enable(intr_enable),
                                                                                    m_pin_mode(pin_mode),
                                                                                    m_pull_mode(pull_mode)
        {
            gpio_reset_pin(Pin);
            esp_err_t direction_err = gpio_set_direction(Pin, m_pin_mode);
            esp_err_t mode_err = gpio_set_pull_mode(Pin, m_pull_mode);
            esp_err_t intr_err;

            if (m_intr_enable) {
                intr_err = gpio_intr_enable(Pin);
            } else {
                intr_err = gpio_intr_disable(Pin);
            }

            if (!(direction_err == ESP_OK && mode_err == ESP_OK && intr_err == ESP_OK)) m_active = false;
        };

        ~Gpio() {
            gpio_reset_pin(Pin);
        };

        int32_t level() {
            return gpio_get_level(Pin);
        };
    private:
        bool m_active;
        bool m_intr_enable;
        gpio_mode_t m_pin_mode;
        gpio_pull_mode_t m_pull_mode;
};

template <uint64_t check_interval>
class Timer {
    public:
        Timer(std::string name):    m_can_run(false),
                                    m_is_running(false)
        {
            m_timer_conf.name = name.c_str();
            m_timer_conf.dispatch_method = ESP_TIMER_TASK;
        };

        ~Timer() {
            m_can_run = false;
        };

        void registerCallback(esp_timer_cb_t callback, void* arg) {
            m_timer_conf.callback = callback;
            m_timer_conf.arg = arg;

            ESP_ERROR_CHECK(esp_timer_create(&m_timer_conf, &m_timer_handle));

            m_can_run = true;
        };

        void startTimerPeriodic() {
            if (m_can_run) {
                ESP_ERROR_CHECK(esp_timer_start_periodic(m_timer_handle, check_interval));
                m_is_running = true;
            }
        };

        void startTimerOneshot(uint64_t timeout) {
            if (m_can_run) {
                ESP_ERROR_CHECK(esp_timer_start_once(m_timer_handle, timeout));
                m_is_running = true;
            }
        };
    private:
        esp_timer_handle_t m_timer_handle;
        esp_timer_create_args_t m_timer_conf;
        bool m_can_run;
        bool m_is_running;
};

template <gpio_num_t Pin, uint32_t frequency>
class PwmDriver {
    public:
        PwmDriver() {
            // Setup PWM signal for diode (see: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html#ledc-api-configure-channel)
            // Configure timer 
            ledc_timer_config_t diode_timer_conf;
            diode_timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
            diode_timer_conf.duty_resolution = LEDC_TIMER_8_BIT;
            diode_timer_conf.timer_num = LEDC_TIMER_0;
            diode_timer_conf.freq_hz = frequency;
            diode_timer_conf.clk_cfg = LEDC_AUTO_CLK;

            ESP_ERROR_CHECK(ledc_timer_config(&diode_timer_conf));

            // Configure channel
            ledc_channel_config_t channel_conf;
            channel_conf.gpio_num = Pin;
            channel_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
            channel_conf.channel = LEDC_CHANNEL_0;
            channel_conf.intr_type = LEDC_INTR_DISABLE;
            channel_conf.timer_sel = LEDC_TIMER_0;
            channel_conf.duty = 200;
            channel_conf.hpoint = 0;

            ESP_ERROR_CHECK(ledc_channel_config(&channel_conf));
        };

        ~PwmDriver() {};
};

template<typename M, uint8_t Length>
class Queue {
    public:
        Queue(uint32_t default_timeout): m_timeout(default_timeout) {
            m_queue = xQueueCreate(Length, sizeof(M));
            m_usable = m_queue == 0 ? false : true;
        };

        ~Queue() {
            vQueueDelete(m_queue);
        };

        void sendToFront(M message) {
            xQueueSendToFront(m_queue, message, static_cast<TickType_t>(m_timeout));
        };

        void sendToBack(M message) {
            xQueueSendToBack(m_queue, message, static_cast<TickType_t>(m_timeout));
        };

        M receive() {
            M m;
            xQueueReceive(m_queue, m, static_cast<TickType_t>(m_timeout));
            return m;
        };
    private:
        QueueHandle_t m_queue;
        bool m_usable;
        uint32_t m_timeout;
};

#endif