#include <utility>

#include "esp_log.h"
#include "esp_spiffs.h"

#include "persistent_store.h"

static const char* TAG = "PersistentStore";

size_t spiffsFreeSpace(const char* partition_label) {
    size_t used = 0;
    size_t total = 0;
    ESP_ERROR_CHECK(esp_spiffs_info(partition_label, &total, &used));

    return total - used;
}

PersistentStore::PersistentStore() {
    ESP_LOGV(TAG, "Persistent store ctor called");

    m_spiffs_conf.base_path = "/spiffs";
    m_spiffs_conf.max_files = 1;
    m_spiffs_conf.partition_label = NULL;
    m_spiffs_conf.format_if_mount_failed = true;

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&m_spiffs_conf));

    size_t total = 0;
    size_t used = 0;
    ESP_ERROR_CHECK(esp_spiffs_info(m_spiffs_conf.partition_label, &total, &used));

    ESP_LOGV(TAG, "Partition size: Total -> %d, Used -> %d", total, used);

    FILE* file = fopen("/spiffs/log", "a+");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file");
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
        fprintf(m_log_file, "%lld%d", pass->time, pass->duration);
    }
}

void PersistentStore::printLog() {
    // Save initial position
    fpos_t initial_position;
    fgetpos(m_log_file, &initial_position);

    size_t length = ftell(m_log_file);

    // Print all entries between start and initial position
    for (auto i = 0; i <= length / (sizeof(int64_t) + sizeof(uint16_t)); i += (sizeof(int64_t) + sizeof(uint16_t))) {
        int64_t time;
        uint16_t duration;

        // Jump to start of pair
        fseek(m_log_file, i, SEEK_SET);
        // Read time from log file
        fread(&time, sizeof(time), 1, m_log_file);

        // Jump to duration
        fseek(m_log_file, i + 8, SEEK_SET);
        // Read duration

        fread(&duration, sizeof(duration), 1, m_log_file);

        printf("Pass: Time -> %lld, Duration -> %d", time, duration);
    }

    // Restore initial position
    fsetpos(m_log_file, &initial_position);
}