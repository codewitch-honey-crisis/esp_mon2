#pragma once
#include <stdint.h>
struct read_status {
    constexpr static const int command = 1;
    int cpu_usage;
    int cpu_temp;
    int cpu_temp_max;
    int gpu_usage;
    int gpu_temp;
    int gpu_temp_max;
};