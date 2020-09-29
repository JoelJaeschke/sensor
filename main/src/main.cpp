#include <stdio.h>
#include <memory>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "sensor_reader.hpp"
#include "config_manager.hpp"
#include "persistent_store.hpp"
#include "ipc_struct.hpp"

static const char* TAG = "[Main]";

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
            ESP_LOGD("[Main - SensorReader]", "Pass registered at %lld wit duration of %d milliseconds", pass->time, pass->duration);
            xQueueSendToFront(config->passQueue, &pass.value(), 10);
        }

        // Yield
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void pass_store_task(void* param) {
    ConfigManager* config = reinterpret_cast<ConfigManager*>(param);

    ESP_LOGI(TAG, "Initializing persistent store");
    PersistentStore store;
    Pass pass;

    ESP_LOGD("[Main - PersistentStore]", "Printing previous passes");
    #if CONFIG_LOG_DEFAULT_LEVEL >= 4
        store.printLog();
    #endif

    for (;;) {
        if (xQueueReceive(config->passQueue, &pass, 10)) {
            ESP_LOGD("[Main - PersistentStore]", "Adding pass: Time -> %lld, duration -> %d", pass.time, pass.duration);
            store.addPass(&pass);
            ESP_LOGD("[Main - PersistentStore]", "Added pass");
        };

        // Yield
        store.printLog();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    };
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
    // auto queue = Queue<Pass, 10>(10);
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
    
    // Pin store task to core 0
    xTaskCreatePinnedToCore(
        pass_store_task,
        "PassStoreTask",
        10000,
        reinterpret_cast<void*>(config),
        1,
        NULL,
        1
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