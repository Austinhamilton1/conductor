#pragma once

#include "Query.h"
#include "core/SimulationClock.h"
#include "commands/CommandQueue.h"

namespace conductor {
    template<typename QueryT>
    class SystemContext {
    public:
        SystemContext() = default;

        SystemContext(
            QueryT& query,
            CommandQueue& commands,
            const SimulationClock& clock,
            double step
        ) : m_query(query),
            m_commands(commands),
            m_clock(clock),
            m_step(step) {}

        QueryT& query() {
            return m_query;
        }

        CommandQueue& commands() {
            return m_commands;
        }

        const SimulationClock& clock() const {
            return m_clock;
        }

        double step() const {
            return m_step;
        }

    private:
        QueryT m_query;
        CommandQueue m_commands;
        SimulationClock m_clock;
        double m_step;
    };
}