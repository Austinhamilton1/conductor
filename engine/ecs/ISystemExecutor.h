#pragma once

namespace conductor {
    class CommandQueue;
    class SimulationClock;

    class ISystemExecutor {
    public:
        virtual ~ISystemExecutor() = default;

        virtual void execute(CommandQueue& commands, double delta_seconds, Random& rand) = 0;
    };
}