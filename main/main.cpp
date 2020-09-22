#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sensor_reader.h"

extern "C" {
    void app_main();
}

void sensor_reader_task(void* param) {
    SensorReader sr;
    sr.setup();

    for (;;) {
        sr.process();
        
        // Yield
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void default_feed_task(void* param) {
    for (;;) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    printf("Starting sensor node!\n");

    // Pin Sensor reading to core 0
    xTaskCreatePinnedToCore(
        sensor_reader_task,
        "SensorReaderTask",
        10000,
        NULL,
        1,
        NULL,
        0
    );
    
    // Create default Task to avoid watchdog starvation
    xTaskCreatePinnedToCore(
        default_feed_task,
        "DefaultFeedTask",
        1000,
        NULL,
        1,
        NULL,
        1
    );
}