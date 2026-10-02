#pragma once

namespace conductor {
    class CommandQueue;
    class SimulationClock;
    class Random;
    class EventBus;
    class Executor;

    class ISystemExecutor {
    public:
        virtual ~ISystemExecutor() = default;

        virtual void execute(
            CommandQueue& commands,
            const SimulationClock& clock,
            Random& rand,
            EventBus& events,
            Executor& executor,
            double delta_seconds
        ) = 0;
    };
}