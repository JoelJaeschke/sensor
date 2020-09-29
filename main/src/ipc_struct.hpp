#ifndef __IPC_STRUCT_H
#define __IPC_STRUCT_H

#include <cstdint>

typedef struct Pass {
    int64_t time;
    uint16_t duration;
} Pass;

#endif