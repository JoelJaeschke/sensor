#include "esp_log.h"

#include "config_manager.hpp"

static const char* TAG = "ConfigManager";

ConfigManager::ConfigManager() {
    ESP_LOGV(TAG, "ConfigManager ctor called");
};
ConfigManager::~ConfigManager() {
    ESP_LOGV(TAG, "ConfigManager dtor called");
};