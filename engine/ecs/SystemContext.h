#pragma once

#include "Query.h"
#include "core/SimulationClock.h"
#include "core/Random.h"
#include "commands/CommandQueue.h"

namespace conductor {
    template<typename BoundQueryT>
    class SystemContext {
    public:
        using query_type = BoundQueryT;

        SystemContext(
            BoundQueryT& query,
            CommandQueue& commands,
            double delta_seconds,
            Random& rand
        ) : m_query(query),
            m_commands(commands),
            m_delta_seconds(delta_seconds),
            m_rand(rand) {}

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

        double delta_seconds() const noexcept {
            return m_delta_seconds;
        }

        Random& rand() noexcept {
            return m_rand;
        }

        const Random& rand() const noexcept {
            return m_rand;
        }

    private:
        BoundQueryT& m_query;
        CommandQueue& m_commands;
        double m_delta_seconds;
        Random& m_rand;
    };
}