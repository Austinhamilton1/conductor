#pragma once

#include "core/Random.h"
#include "core/SimulationClock.h"
#include "commands/CommandQueue.h"
#include "events/EventBus.h"

namespace conductor {
    struct WorldContext {
        Random& rand;
        SimulationClock& clock;
        CommandQueue& queue;
        EventBus& events;
    };
}
