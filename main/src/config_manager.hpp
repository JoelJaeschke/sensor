#ifndef __CONFIG_MANAGER_H
#define __CONFIG_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "wrapper.hpp"

class ConfigManager {
    public:
        ConfigManager();
        ~ConfigManager();

        Queue<Pass, 5> passQueue;
};

#endif