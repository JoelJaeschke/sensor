#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "sensor_reader.h"
#include "config_manager.h"

extern "C" {    
    void app_main();
}

// --- Task implementation --- //
void sensor_reader_task(void* param) {
    ConfigManager* config = reinterpret_cast<ConfigManager*>(param);

    SensorReader sr;
    sr.setup();

    for (;;) {
        sr.process();
        
        uint64_t num_passes = sr.getNumberOfPasses();
        xQueueSendToFront(config->passQueue, &num_passes, 10);

        // Yield
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void pass_printer_task(void* param) {
    ConfigManager* config = reinterpret_cast<ConfigManager*>(param);
    uint64_t previous_pass_num = 0;

    for (;;) {
        uint64_t num_passes;
        xQueueReceive(config->passQueue, &num_passes, 10);

        if (num_passes > previous_pass_num) {
            previous_pass_num = num_passes;
            printf("Printer: Number of passes is %lld.\n", num_passes);
        }

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

    QueueHandle_t passQueue = xQueueCreate(10, sizeof(uint64_t));

    ConfigManager* config = new ConfigManager();
    config->passQueue = passQueue;
    
    // Pin Sensor reading to core 0
    xTaskCreatePinnedToCore(
        sensor_reader_task,
        "SensorReaderTask",
        10000,
        reinterpret_cast<void*>(config),
        1,
        NULL,
        0
    );

    // Pin printer task to core 0
    xTaskCreatePinnedToCore(
        pass_printer_task,
        "PassPrinterTask",
        10000,
        reinterpret_cast<void*>(config),
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