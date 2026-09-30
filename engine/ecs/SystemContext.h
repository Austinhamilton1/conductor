#pragma once

#include "Query.h"
#include "core/SimulationClock.h"
#include "core/Random.h"
#include "commands/CommandQueue.h"
#include "events/EventBus.h"

namespace conductor {
    template<typename BoundQueryT>
    class SystemContext {
    public:
        using query_type = BoundQueryT;

        SystemContext(
            BoundQueryT& query,
            CommandQueue& commands,
            double delta_seconds,
            SimulationClock& clock,
            Random& rand,
            EventBus& events
        ) : m_query(query),
            m_commands(commands),
            m_delta_seconds(delta_seconds),
            m_clock(clock),
            m_rand(rand),
            m_events(events) {}

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

        const SimulationClock& clock() const noexcept {
            return m_clock;
        }

        Random& rand() noexcept {
            return m_rand;
        }

        const Random& rand() const noexcept {
            return m_rand;
        }

        template<typename T>
        void publish(T&& event) {
            m_events.publish<T>(event);
        }

    private:
        BoundQueryT& m_query;
        CommandQueue& m_commands;
        double m_delta_seconds;
        SimulationClock& m_clock;
        Random& m_rand;
        EventBus& m_events;
    };
}