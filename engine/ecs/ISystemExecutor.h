#pragma once

namespace conductor {
    class CommandQueue;
    class SimulationClock;
    class Random;
    class EventBus;

    class ISystemExecutor {
    public:
        virtual ~ISystemExecutor() = default;

        virtual void execute(
            CommandQueue& commands,
            SimulationClock& clock,
            Random& rand,
            EventBus& events,
            double delta_seconds
        ) = 0;
    };
}