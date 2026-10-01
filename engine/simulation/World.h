#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

#include "WorldContext.h"
#include "core/NonCopyable.h"
#include "core/SimulationClock.h"
#include "core/Random.h"
#include "ecs/ComponentType.h"
#include "ecs/SystemType.h"
#include "ecs/Entity.h"
#include "ecs/EntityManager.h"
#include "ecs/ComponentRegistry.h"
#include "ecs/ComponentStorageRegistry.h"
#include "ecs/Query.h"
#include "events/EventBus.h"
#include "commands/CommandQueue.h"
#include "scheduler/Scheduler.h"
#include "runtime/ApplicationContext.h"

namespace conductor {
    class World;
    
    struct SpawnContext {
        Entity entity;
        std::size_t index;
        Random& random;
    };

    class SpawnConfig {
    public:
        template<Component T>
        SpawnConfig& component(T value);

        template<Component T, typename F>
        requires std::invocable<F&, const SpawnContext&> &&
                 std::convertible_to<
                    std::invoke_result_t<F&, const SpawnContext&>,
                    T
                 >
        SpawnConfig& component(F initializer);

        void initialize(World& world, Entity entity, SpawnContext& context) const;

    private:
        std::vector<std::function<void(World&, Entity, SpawnContext&)>> m_initializers;
    };
    
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
        World(Random& rand) 
            : m_rand(rand),
              m_commands(), 
              m_scheduler() {};
        
        ~World() {};

        // Configuration
        template<Component T>
        void register_component();

        template<Component T>
        void add_component(Entity entity, T& component);

        template<Component T>
        T *get_component(Entity entity);

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

        void register_spawner(const std::string& name, SpawnConfig config);
        void spawn_entities(std::size_t count, const std::string& name);

        void update(double delta, ApplicationContext ctx);

        WorldContext get_context(ApplicationContext ctx);

        // Access
        EntityManager& entities();
        const EntityManager& entities() const;

        ComponentRegistry& components();
        const ComponentRegistry& components() const;

        ComponentStorageRegistry& storage();
        const ComponentStorageRegistry& storage() const;

    private:
        Random& m_rand;
        EntityManager m_entity_manager;
        ComponentRegistry m_components;
        ComponentStorageRegistry m_storages;
        CommandQueue m_commands;
        Scheduler m_scheduler;
        std::vector<Entity> m_entities;
        std::unordered_map<std::string, SpawnConfig> m_spawners;

        bool m_finalized{false};
    };

    template<Component T>
    SpawnConfig& SpawnConfig::component(T value) {
        m_initializers.emplace_back(
            [value = std::move(value)](
                World& world,
                Entity entity,
                SpawnContext&
            ) mutable {
                world.add_component<T>(entity, value);
            }
        );

        return *this;
    }

    template<Component T, typename F>
    requires std::invocable<F&, const SpawnContext&> &&
             std::convertible_to<
                std::invoke_result_t<F&, const SpawnContext&>,
                T
             >
    SpawnConfig& SpawnConfig::component(F initializer) {
        m_initializers.emplace_back(
            [initializer = std::move(initializer)](
                World& world,
                Entity entity,
                SpawnContext& context
            ) mutable {
                T value = initializer(context);
                world.add_component<T>(entity, value);
            }
        );

        return *this;
    }

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

    template<Component T>
    T *World::get_component(Entity entity) {
        auto& storage = m_storages.get<T>();
        return storage.get(entity);
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