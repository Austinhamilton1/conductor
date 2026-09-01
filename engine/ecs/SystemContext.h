#pragma once

#include "Query.h"
#include "core/SimulationClock.h"
#include "commands/CommandQueue.h"

namespace conductor {
    template<typename BoundQueryT>
    class SystemContext {
    public:
        using query_type = BoundQueryT;

        SystemContext(
            BoundQueryT& query,
            CommandQueue& commands,
            SimulationClock& clock
        ) : m_query(query),
            m_commands(commands),
            m_clock(clock) {}

        BoundQueryT& query() noexcept {
            return m_query;
        }

        const BoundQueryT& query() const noexcept {
            return m_query;
        }

        CommandQueue& commands() noexcept {
            return m_commands;
        }

        const CommandQueue& commands() const noexcept {
            return m_commands;
        }

        SimulationClock& clock() noexcept {
            return m_clock;
        }

        const SimulationClock& clock() const noexcept {
            return m_clock;
        }

    private:
        BoundQueryT& m_query;
        CommandQueue& m_commands;
        SimulationClock& m_clock;
    };
}