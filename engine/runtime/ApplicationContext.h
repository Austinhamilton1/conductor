#pragma once 

#include "core/Random.h"
#include "core/SimulationClock.h"
#include "events/EventBus.h"

namespace conductor {
    struct ApplicationContext {
        Random& random;
        SimulationClock& clock;
        EventBus& events;
    };
}