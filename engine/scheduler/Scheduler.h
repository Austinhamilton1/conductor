#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "DependencyResult.h"
#include "SystemDependency.h"
#include "SystemId.h"
#include "SystemSchedule.h"
#include "core/NonCopyable.h"
#include "ecs/SystemType.h"
#include "ecs/SystemExecutor.h"
#include "ecs/Query.h"

namespace conductor {  
    class CommandQueue;
    class SimulationClock;

    class Scheduler {
    public:
        class SystemHandle : public NonCopyable {
        public:
            SystemHandle() = default;

            SystemHandle(Scheduler *scheduler, uint64_t id)
                : m_scheduler(scheduler),
                  m_id(id) {};

            ~SystemHandle();

            SystemHandle(SystemHandle&& other) noexcept;
            SystemHandle& operator=(SystemHandle&& other) noexcept;

            void cancel();
            void pause();
            void resume();

            void set_frequency(double frequency);
            bool valid() const;

            SystemId id() const;
            uint32_t generation() const;
            SystemState state() const;
            bool paused() const;

            DependencyResult depends_on(const SystemHandle& parent);

        private:
            Scheduler *m_scheduler = nullptr;
            uint64_t m_id = 0;
        };

        Scheduler(
            CommandQueue& commands,
            SimulationClock& clock
        ) noexcept
            : m_commands(commands),
              m_clock(clock) {}

        template<System T, typename BoundQueryT>
        SystemHandle schedule(
            std::string name,
            double frequency,
            T system,
            BoundQueryT query
        ) {
            if(frequency <= 0.0) {
                throw std::invalid_argument(
                    "System frequency must be greater than zero"
                );
            }

            using QueryT = typename BoundQueryT::query_type;

            const SystemId id = m_next_id++;

            const double interval = 1.0 / frequency;

            auto executor =
                std::make_unique<SystemExecutor<T, BoundQueryT>>(
                    std::move(system),
                    std::move(query)
                );

            auto accesses = query_accesses<QueryT>();

            m_systems.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(id),
                std::forward_as_tuple(
                    SystemSchedule(
                        std::move(name),
                        interval
                    ),
                    std::move(executor),
                    std::move(accesses)
                )
            );

            m_dependencies_resolved = false;

            return SystemHandle(this, id);
        }

        DependencyResult depends_on(
            SystemId child,
            SystemId parent
        );

        void update(double delta);
        void resolve_dependencies();

        bool has_dependency(
            SystemId child,
            SystemId parent
        ) const;

        bool has_dependency_path(
            SystemId child,
            SystemId parent
        ) const noexcept;

    private:
        struct SystemNode {
            SystemSchedule schedule;

            std::unique_ptr<ISystemExecutor> executor;

            std::vector<ComponentAccess> accesses;

            std::vector<SystemDependency> dependencies;
            std::vector<SystemId> dependents;

            SystemNode(
                SystemSchedule schedule,
                std::unique_ptr<ISystemExecutor> executor,
                std::vector<ComponentAccess> accesses
            )
                : schedule(std::move(schedule)),
                  executor(std::move(executor)),
                  accesses(std::move(accesses)) {}
        };

        struct InferredCycle {
            SystemId first;
            SystemId second;
            bool resolved = false;
        };

        SystemId m_next_id = 1;
        
        std::unordered_map<SystemId, SystemNode> m_systems;
        std::vector<InferredCycle> m_inferred_cycles;

        CommandQueue& m_commands;
        SimulationClock& m_clock;

        bool m_dependencies_resolved = false;

    private:
        void remove(SystemId id);
        void pause(SystemId id);
        void resume(SystemId id);

        void set_frequency(
            SystemId id,
            double frequency
        );

        bool dependencies_ready(SystemId id) const;

        bool add_inferred_dependency(
            SystemId child,
            SystemId parent
        );

        void infer_dependency(
            SystemId first,
            SystemId second
        );

        void infer_dependencies();

        void validate_write_conflicts() const;

        bool creates_cycle(
            SystemId child,
            SystemId parent
        ) const;

        void consume_dependencies(SystemId id);

        bool execute_system(SystemId id);

        bool remove_dependency(
            SystemId child,
            SystemId parent
        );

        bool has_inferred_cycle(
            SystemId first,
            SystemId second
        ) const;

        bool has_unresolved_inferred_cycle(
            SystemId first,
            SystemId second
        ) const;

        DependencyResult add_inferred_dependency_edge(
            SystemId child,
            SystemId parent
        );

        bool is_inferred_dependency(
            SystemId child,
            SystemId parent
        ) const;

    private:
        template<typename... Access>
        std::vector<ComponentAccess> query_accesses_impl(std::tuple<Access...>*) {
            std::vector<ComponentAccess> accesses;
            accesses.reserve(sizeof...(Access));

            (
                accesses.push_back(
                    ComponentAccess{
                        ComponentRegistry::register_component<
                            typename Access::component_type
                        >(),
                        Access::writable
                            ? AccessMode::Write
                            : AccessMode::Read
                    }
                ),
                ...
            );

            return accesses;
        }

        template<typename QueryT>
        requires QueryType<QueryT>
        std::vector<ComponentAccess> query_accesses() {
            using AccessTypes = typename QueryT::access_types;

            return query_accesses_impl(
                static_cast<AccessTypes*>(nullptr)
            );
        }
    };
}