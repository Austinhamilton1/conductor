#pragma once

#include <memory>
#include <utility>

#include "ISystemExecutor.h"
#include "SystemType.h"
#include "SystemContext.h"

namespace conductor {
    template<System T, typename BoundQueryT>
    requires requires(
        T& system,
        SystemContext<BoundQueryT>& context
    ) {
        {
            system.update(context)
        } -> std::same_as<void>;
    }
    class SystemExecutor final : public ISystemExecutor {
    public:
        SystemExecutor(
            T system,
            BoundQueryT query
        )
            : m_system(std::move(system)),
              m_query(std::move(query)) {}

        void execute(
            CommandQueue& commands,
            const SimulationClock& clock,
            Random& rand,
            EventBus& events,
            double delta_seconds
        ) override {
            SystemContext<BoundQueryT> context(
                m_query,
                commands,
                delta_seconds,
                clock,
                rand,
                events
            );

            m_system.update(context);
        }

        T& system() noexcept {
            return m_system;
        }

        const T& system() const noexcept {
            return m_system;
        }

        BoundQueryT& query() noexcept {
            return m_query;
        }

        const BoundQueryT& query() const noexcept {
            return m_query;
        }

    private:
        T m_system;
        BoundQueryT m_query;
    };
}