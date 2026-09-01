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
            SimulationClock& clock
        ) override {
            SystemContext<BoundQueryT> context(
                m_query,
                commands,
                clock
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