#pragma once

#include <stdexcept>
#include <string>
#include <utility>

#include "core/NonCopyable.h"
#include "core/SimulationClock.h"
#include "ecs/ComponentType.h"
#include "ecs/SystemType.h"
#include "ecs/Entity.h"
#include "ecs/EntityManager.h"
#include "ecs/ComponentRegistry.h"
#include "ecs/ComponentStorageRegistry.h"
#include "ecs/Query.h"
#include "commands/CommandQueue.h"
#include "scheduler/Scheduler.h"

namespace conductor {
    namespace detail {
        template<typename Query>
        struct WorldQueryBinder;

        template<typename... Access>
        struct WorldQueryBinder<Query<Access...>> {
            static auto bind(ComponentStorageRegistry& storages) {
                return Query<Access...>::bind_storage(
                    storages.get<typename Access::component_type>()...
                );
            }
        };
    }

    class World : public NonCopyable {
    public:
        World() 
            : m_clock(), 
              m_commands(), 
              m_scheduler(m_commands, m_clock) {};
        
        ~World() {};

        // Configuration
        template<Component T>
        void register_component();

        template<Component T>
        void add_component(Entity entity, T& component);

        template<System T>
        Scheduler::SystemHandle add_system(
            std::string name,
            double frequency,
            T system
        );

        void finalize();

        // Simulation
        Entity create_entity();
        Entity get_entity(size_t index);
        void destroy_entity(Entity entity);

        void update(double delta);

        // Access
        EntityManager& entities();
        const EntityManager& entities() const;

        ComponentRegistry& components();
        const ComponentRegistry& components() const;

        ComponentStorageRegistry& storage();
        const ComponentStorageRegistry& storage() const;

        SimulationClock& clock();
        const SimulationClock& clock() const;

    private:
        EntityManager m_entity_manager;
        ComponentRegistry m_components;
        ComponentStorageRegistry m_storages;
        SimulationClock m_clock;
        CommandQueue m_commands;
        Scheduler m_scheduler;
        std::vector<Entity> m_entities;

        bool m_finalized{false};
    };

    template<Component T>
    void World::register_component() {
        if(m_finalized) {
            throw std::logic_error(
                "Cannot register component after world finalization"
            );
        }

        m_components.register_component<T>();
        m_storages.create<T>();
    }

    template<Component T>
    void World::add_component(Entity entity, T& component) {
        if(m_finalized) {
            throw std::logic_error(
                "Cannot add component after world finalization"
            );
        }

        ComponentStorage<T>& storage = m_storages.get<T>();
        storage.add(entity, component);
    }

    template<System T>
    Scheduler::SystemHandle World::add_system(
        std::string name,
        double frequency,
        T system
    ) {
        if(m_finalized) {
            throw std::logic_error(
                "Cannot add system after world finalization"
            );
        }

        using QueryType = typename T::Query;

        auto query = detail::WorldQueryBinder<QueryType>::bind(
            m_storages
        );

        return m_scheduler.schedule(
            std::move(name),
            frequency,
            std::move(system),
            std::move(query)
        );
    }
}