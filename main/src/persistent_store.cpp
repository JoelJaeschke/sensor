#include <utility>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "persistent_store.hpp"

#define RESET true

size_t spiffsFreeSpace(const char* partition_label) {
    size_t used = 0;
    size_t total = 0;
    ESP_ERROR_CHECK(esp_spiffs_info(partition_label, &total, &used));

    return total - used;
}

PersistentStore::PersistentStore(bool reset):   m_can_recover(false),
                                                m_reset(reset)
{
    ESP_LOGD("PersistentStore", "Persistent store ctor called");

    m_spiffs_conf.base_path = "/spiffs";
    m_spiffs_conf.max_files = 1;
    m_spiffs_conf.partition_label = NULL;
    m_spiffs_conf.format_if_mount_failed = true;

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&m_spiffs_conf));

    size_t total = 0;
    size_t used = 0;
    ESP_ERROR_CHECK(esp_spiffs_info(m_spiffs_conf.partition_label, &total, &used));

    ESP_LOGD("PersistentStore", "Partition size: Total -> %d, Used -> %d", total, used);

    if (m_reset) {
        unlink("/spiffs/log");
    }

    struct stat st;
    if (stat("/spiffs/log", &st) == 0) {
        m_can_recover = true;
    }

    const char* flags = m_can_recover ? "a+b" : "w+b";
    ESP_LOGD("PersistentStore", "Can recover: %s. Setting flags: %s", m_can_recover ? "yes": "no", flags);
    FILE* file = fopen("/spiffs/log", flags);
    if (file == NULL) {
        ESP_LOGE("PersistentStore", "Failed to open file");
    }

    m_log_file = file;
}

PersistentStore::~PersistentStore() {
    fclose(m_log_file);
    esp_vfs_spiffs_unregister(m_spiffs_conf.partition_label);
}

void PersistentStore::addPass(Pass* pass) {
    bool is_mounted = esp_spiffs_mounted(m_spiffs_conf.partition_label);
    size_t free = spiffsFreeSpace(m_spiffs_conf.partition_label);
    bool space_left = free >= sizeof(int64_t) + sizeof(uint16_t);

    if (is_mounted && space_left) {
        fwrite(&pass->time, sizeof(pass->time), 1, m_log_file);
        fwrite(&pass->duration, sizeof(pass->duration), 1, m_log_file);
    }
}

#if CONFIG_LOG_DEFAULT_LEVEL == 4
    void PersistentStore::printLog() {
        // Save initial position
        fpos_t initial_position;
        fgetpos(m_log_file, &initial_position);

        ESP_LOGD("PersistentStore", "Current position: %ld", initial_position);

        size_t length = ftell(m_log_file);

        if (length == 0) {
            ESP_LOGD("PersistentStore", "Log file is empty, aborting print");
            return;
        }

        ESP_LOGD("PersistentStore", "File length: %u", length);
        ESP_LOGD("PersistentStore", "Limits: %u", length);
        ESP_LOGD("PersistentStore", "Increment: %u", (sizeof(int64_t) + sizeof(uint16_t)));

        // Print all entries between start and initial position
        for (auto i = 0; i <= length; i += (sizeof(int64_t) + sizeof(uint16_t))) {
            int64_t time;
            uint16_t duration;

            // Jump to start of pair
            fseek(m_log_file, i, SEEK_SET);
            // Read time from log file
            auto time_read = fread(&time, sizeof(time), 1, m_log_file);

            // Jump to duration
            fseek(m_log_file, i + 8, SEEK_SET);
            
            // Read duration
            fread(&duration, sizeof(duration), 1, m_log_file);

            ESP_LOGD("PersistentStore", "Pass: Time -> %lld, Duration -> %d\n", time, duration);
        }

        // Restore initial position
        fsetpos(m_log_file, &initial_position);
    }
#endif