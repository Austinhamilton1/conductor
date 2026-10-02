#pragma once

#include "core/Executor.h"
#include "core/Random.h"
#include "core/SimulationClock.h"
#include "commands/CommandQueue.h"
#include "events/EventBus.h"

namespace conductor {
    struct WorldContext {
        Random& random;
        const SimulationClock& clock;
        EventBus& events;
        Executor& executor;
        CommandQueue& queue;
    };
}
