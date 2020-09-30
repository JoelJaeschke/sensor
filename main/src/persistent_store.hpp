#ifndef __PERSISTENT_STORE_H
#define __PERSISTENT_STORE_H

#include "esp_spiffs.h"
#include "sdkconfig.h"

#include "ipc_struct.hpp"

class PersistentStore {
    public:
        PersistentStore(bool reset);
        ~PersistentStore();
        void addPass(Pass* pass);
        
        #if CONFIG_LOG_DEFAULT_LEVEL == 4
            void printLog();
        #endif
    private:
        FILE* m_log_file;
        esp_vfs_spiffs_conf_t m_spiffs_conf;
        bool m_can_recover;
        bool m_reset;
};

#endif