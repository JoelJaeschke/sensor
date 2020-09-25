#include <stdio.h>
#include <memory>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "sensor_reader.h"
#include "config_manager.h"
#include "ipc_struct.h"

static const char* TAG = "Main";

extern "C" {    
    void app_main();
}

// --- Task implementation --- //
void sensor_reader_task(void* param) {
    ConfigManager* config = reinterpret_cast<ConfigManager*>(param);

    ESP_LOGI(TAG, "Initializing sensor reader");
    SensorReader sr;

    for (;;) {
        auto pass = sr.process();
        if (pass) {
            ESP_LOGV(TAG, "Pass registered at %lld wit duration of %d milliseconds", pass->time, pass->duration);
            xQueueSendToFront(config->passQueue, &pass.value(), 10);
        }

        // Yield
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void pass_printer_task(void* param) {
    ConfigManager* config = reinterpret_cast<ConfigManager*>(param);

    Pass pass;
    uint16_t previous_duration = 0;

    for (;;) {

        xQueueReceive(config->passQueue, &pass, 10);


        if (pass.duration != previous_duration) {
            previous_duration = pass.duration;
            ESP_LOGV(TAG, "Pass registered at %lld wit duration of %d milliseconds", pass.time, pass.duration);
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
    ESP_LOGI(TAG, "Starting sensor node!");

    ESP_LOGI(TAG, "Initializing queue's");
    QueueHandle_t passQueue = xQueueCreate(5, sizeof(Pass));


    ESP_LOGI(TAG, "Initializing config manager");
    ConfigManager* config = new ConfigManager();
    config->passQueue = passQueue;
    
    ESP_LOGI(TAG, "Initializing task's");
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