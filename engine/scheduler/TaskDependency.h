#pragma once

#include <cstdint>

#include "TaskId.h"

namespace conductor {
    struct TaskDependency {
        TaskId parent;
        uint64_t consumed_generation = 0;
    };
}