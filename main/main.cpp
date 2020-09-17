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
        // TODO: Periodically send number of people who passed by somewhere
    }
}

void app_main(void)
{
    printf("Starting sensor node! Yadda\n");
    
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
}