#pragma once

#include "core/Executor.h"
#include "core/Random.h"
#include "core/SimulationClock.h"
#include "events/EventBus.h"

namespace conductor {
    struct ApplicationContext {
        Random& random;
        const SimulationClock& clock;
        EventBus& events;
        Executor& executor;
    };
}