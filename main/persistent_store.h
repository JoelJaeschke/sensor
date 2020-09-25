#ifndef __PERSISTENT_STORE_H
#define __PERSISTENT_STORE_H

#include "esp_spiffs.h"

#include "ipc_struct.h"

class PersistentStore {
    public:
        PersistentStore();
        ~PersistentStore();
        void addPass(Pass* pass);
        void printLog();
    private:
        FILE* m_log_file;
        esp_vfs_spiffs_conf_t m_spiffs_conf;
};

#endif