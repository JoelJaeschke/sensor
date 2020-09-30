#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "sensor_reader.hpp"
#include "config_manager.hpp"
#include "persistent_store.hpp"
#include "ipc_struct.hpp"
#include "wrapper.hpp"

extern "C" {    
    void app_main();
}

/*********************************/
/*      Task implementation      */
/*********************************/
void sensor_reader_task(void* param) {
    ESP_LOGI("Main", "Initializing sensor reader");
    
    // Initialize sensor reader and config
    SensorReader sr;
    ConfigManager* config = reinterpret_cast<ConfigManager*>(param);

    // Task loop
    for (;;) {
        auto pass = sr.process();

        if (pass) {
            ESP_LOGD("Main - SensorReader", "Pass registered at %lld wit duration of %d milliseconds", pass->time, pass->duration);
            config->passQueue.sendToFront(&pass.value());
        }

        // Yield
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void pass_store_task(void* param) {
    ESP_LOGI("Main", "Initializing persistent store");
    
    ConfigManager* config = reinterpret_cast<ConfigManager*>(param);
    std::optional<Pass> pass;
    #if CONFIG_LOG_DEFAULT_LEVEL == 4
        PersistentStore store(true);
    #else
        PersistentStore store(false);
    #endif

    #if CONFIG_LOG_DEFAULT_LEVEL == 4
        ESP_LOGD("Main - PersistentStore", "Printing previous passes");
        store.printLog();
    #endif

    // Task loop
    for (;;) {
        pass = config->passQueue.receive();

        if (pass) {
            ESP_LOGD("Main - PersistentStore", "Adding pass: Time -> %lld, duration -> %d", pass->time, pass->duration);
            store.addPass(&pass.value());
        };

        #if CONFIG_LOG_DEFAULT_LEVEL == 4
            store.printLog();
        #endif

        // Yield
        vTaskDelay(200 / portTICK_PERIOD_MS);
    };
}

// This task is only used to keep watchdog from complaining
// If it is not run, watchdog will yield starvation since only IDLE task is running (persumably)
void default_feed_task(void* param) {
    for (;;) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_LOGI("Main", "Starting sensor node!");

    ESP_LOGI("Main", "Initializing config manager");
    ConfigManager* config = new ConfigManager();
    
    ESP_LOGI("Main", "Initializing task's");
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