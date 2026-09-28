#pragma once

#include <cstdint>

namespace conductor {
    struct ApplicationConfig {
        double duration;
        double time_scale;
        double step_size;
        uint64_t seed;
    };
}