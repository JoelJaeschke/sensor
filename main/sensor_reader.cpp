#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sensor_reader.h"

// --- Private implementation --- //
static void entering_sensor(void* arg) {
    SensorReader* sr = (SensorReader*)arg;
    sr->start_pass();
}

static void leaving_sensor(void* arg) {
    SensorReader* sr = (SensorReader*)arg;
    sr->end_pass();
}

// --- Public implementation --- //
SensorReader::SensorReader():   m_num_passes(0), 
                                m_start_pass(0), 
                                m_end_pass(0),
                                m_pass_complete(true) {};

SensorReader::~SensorReader() {};

void SensorReader::setup() {
    // Setup receiver pin for falling interrupt
    gpio_reset_pin(RECEIVER_PIN_FALLING);
    gpio_set_direction(RECEIVER_PIN_FALLING, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RECEIVER_PIN_FALLING, GPIO_PULLUP_ONLY);
    gpio_intr_enable(RECEIVER_PIN_FALLING);

    // Setup receiver pin for falling interrupt
    gpio_reset_pin(RECEIVER_PIN_RISING);
    gpio_set_direction(RECEIVER_PIN_RISING, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RECEIVER_PIN_RISING, GPIO_PULLUP_ONLY);
    gpio_intr_enable(RECEIVER_PIN_RISING);
    
    // Install ISR Service
    gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
    
    // Register ISR for for receiver state change (falling)
    gpio_set_intr_type(RECEIVER_PIN_FALLING, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(RECEIVER_PIN_FALLING, entering_sensor, (void*)this);

    // Register ISR for for receiver state change (falling)
    gpio_set_intr_type(RECEIVER_PIN_RISING, GPIO_INTR_POSEDGE);
    gpio_isr_handler_add(RECEIVER_PIN_RISING, leaving_sensor, (void*)this);

    // Setup PWM signal for diode (see: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html#ledc-api-configure-channel)
    // Configure timer 
    ledc_timer_config_t timer_conf;
    timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    timer_conf.duty_resolution = LEDC_TIMER_8_BIT;
    timer_conf.timer_num = LEDC_TIMER_0;
    timer_conf.freq_hz = DIODE_FREQUENCY;
    timer_conf.clk_cfg = LEDC_AUTO_CLK;

    ledc_timer_config(&timer_conf);

    // Configure channel
    ledc_channel_config_t channel_conf;
    channel_conf.gpio_num = IR_DIODE_PIN;
    channel_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    channel_conf.channel = LEDC_CHANNEL_0;
    channel_conf.intr_type = LEDC_INTR_DISABLE;
    channel_conf.timer_sel = LEDC_TIMER_0;
    channel_conf.duty = 96;

    ledc_channel_config(&channel_conf);

    return;
}

void SensorReader::process() {
    // If pass is complete, reset and increment passes
    if (m_pass_complete) {
        int64_t delta = m_end_pass - m_start_pass;

        if (delta > PASSING_THRESHOLD) m_num_passes++;

        m_start_pass = 0;
        m_end_pass = 0;
        m_pass_complete = false;
        
        printf("Delta between start and end: %lld\n", m_end_pass - m_start_pass);
        printf("Pass completed!\n");
    }

    // Sleep for 50 milliseconds after each process() call
    vTaskDelay(50 / portTICK_PERIOD_MS);
    
    return;
}

void SensorReader::start_pass() {
    m_start_pass = esp_timer_get_time();
}

void SensorReader::end_pass() {
    m_end_pass = esp_timer_get_time();
    m_pass_complete = true;
}