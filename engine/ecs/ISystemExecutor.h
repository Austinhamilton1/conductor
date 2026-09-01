#pragma once

namespace conductor {
    class CommandQueue;
    class SimulationClock;

    class ISystemExecutor {
    public:
        virtual ~ISystemExecutor() = default;

        virtual void execute(CommandQueue& commands, SimulationClock& clock) = 0;
    };
}