#ifndef __IPC_STRUCT_H
#define __IPC_STRUCT_H

#include <cstdint>

typedef struct Pass {
    uint64_t time;
    uint16_t duration;
} Pass;

#endif