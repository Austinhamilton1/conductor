#pragma once

#include "core/Random.h"
#include "core/SimulationClock.h"
#include "commands/CommandQueue.h"
#include "events/EventBus.h"

#include "taskflow/taskflow.hpp"

namespace conductor {
    struct WorldContext {
        Random& random;
        const SimulationClock& clock;
        EventBus& events;
        tf::Executor& executor;
        CommandQueue& queue;
    };
}
