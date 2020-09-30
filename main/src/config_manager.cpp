#include "esp_log.h"

#include "ipc_struct.hpp"
#include "config_manager.hpp"

ConfigManager::ConfigManager(): passQueue(Queue<Pass, 5>(10))
{
    ESP_LOGD("ConfigManager", "ConfigManager ctor called");
};
ConfigManager::~ConfigManager() {
    ESP_LOGD("ConfigManager", "ConfigManager dtor called");
};