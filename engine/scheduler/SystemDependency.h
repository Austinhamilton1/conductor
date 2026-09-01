#pragma once

#include <cstdint>

#include "SystemId.h"

namespace conductor {
    struct SystemDependency {
        SystemId parent;
        uint64_t consumed_generation = 0;
    };
}