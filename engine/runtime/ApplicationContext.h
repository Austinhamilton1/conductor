#pragma once

#include "core/Clock.h"
#include "core/SimulationClock.h"
#include "events/EventBus.h"
#include "scheduler/Scheduler.h"

namespace conductor {
    struct ApplicationContext {
        Clock& clock;
        SimulationClock& simulation_clock;
        EventBus& events;
        Scheduler& scheduler;
    };
};