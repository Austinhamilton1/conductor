#pragma once

#include "Query.h"
#include "core/Executor.h"
#include "core/SimulationClock.h"
#include "core/Random.h"
#include "commands/CommandQueue.h"
#include "ecs/Entity.h"
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
            const SimulationClock& clock,
            Random& rand,
            EventBus& events,
            Executor& executor
        ) : m_query(query),
            m_commands(commands),
            m_delta_seconds(delta_seconds),
            m_clock(clock),
            m_rand(rand),
            m_events(events),
            m_executor(executor) {}

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

        template<typename Func>
        void query(Func&& lambda) {
            m_query.iter(
                std::forward<Func>(lambda)
            );
        }

        template<typename Func>
        void parallel_query(Func&& lambda) {
            m_query.par_iter(
                m_executor,
                std::forward<Func>(lambda)
            );
        }

    private:
        BoundQueryT& m_query;
        CommandQueue& m_commands;
        double m_delta_seconds;
        const SimulationClock& m_clock;
        Random& m_rand;
        EventBus& m_events;
        Executor& m_executor;
    };
}