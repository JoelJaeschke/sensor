#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"

#include "sensor_reader.h"

// --- Private implementation --- //
static void entering_sensor(void* arg) {
    SensorReader* sr = (SensorReader*)arg;
    sr->start_pass();
}

// --- Public implementation --- //
SensorReader::SensorReader():   m_num_passes(0), 
                                m_start_pass(0), 
                                m_end_pass(0) {};

SensorReader::~SensorReader() {};

void SensorReader::setup() {
    // Setup receiver pin
    gpio_reset_pin(RECEIVER_PIN);
    gpio_set_direction(RECEIVER_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RECEIVER_PIN, GPIO_PULLUP_ONLY);
    gpio_intr_enable(RECEIVER_PIN);

    // Register ISR for for receiver state change
    gpio_set_intr_type(RECEIVER_PIN, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
    gpio_isr_handler_add(RECEIVER_PIN, entering_sensor, (void*)this);

    // Setup PWM signal for diode
    ledc_set_pin(IR_DIODE_PIN, LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, DIODE_FREQUENCY);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 192);

    return;
}

void SensorReader::process() {
    uint32_t lvl = gpio_get_level(RECEIVER_PIN);
    
    // If pin is high and m_end_pass is 0, a pass is over, so we calculate
    // and reset all values
    if (lvl == 1 && m_end_pass == 0) {
        m_end_pass = esp_timer_get_time();
        int64_t duration_pass = m_end_pass - m_start_pass;
        if (duration_pass > PASSING_THRESHOLD) m_num_passes++;

        m_start_pass = 0;
        m_end_pass = 0;
    };

    // Rest of the cases either should not happen or can be safely ignored
    // TODO: Show some error condition when something goes wrong
    
    return;
}

void SensorReader::start_pass() {
    m_start_pass = esp_timer_get_time();
    m_end_pass = 0;
}