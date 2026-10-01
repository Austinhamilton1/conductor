#pragma once

#include "core/Random.h"
#include "core/SimulationClock.h"
#include "events/EventBus.h"
#include "taskflow/taskflow.hpp"

namespace conductor {
    struct ApplicationContext {
        Random& random;
        const SimulationClock& clock;
        EventBus& events;
        tf::Executor& executor;
    };
}