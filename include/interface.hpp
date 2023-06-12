#pragma once
#include <stdint.h>
// the packet to receive
typedef struct read_status {
    // the command id
    constexpr static const int command = 1;
    // cpu usage from 0-100
    int cpu_usage;
    // cpu temp (C)
    int cpu_temp;
    // cpu tjmax
    int cpu_temp_max;
    // gpu usage from 0-100
    int gpu_usage;
    // gpu temp (C)
    int gpu_temp;
    // gpu tjmax
    int gpu_temp_max;
} read_status_t;