#pragma once

#include <functional>
#include <string>

#include "ecs/Entity.h"
#include "ecs/ComponentType.h"
#include "ecs/SystemType.h"
#include "events/Event.h"
#include "events/EventBus.h"
#include "events/Subscription.h"
#include "scheduler/Scheduler.h"
#include "simulation/SpawnerType.h"
#include "simulation/World.h"

namespace conductor {
    class PluginConfig {
    public:
        PluginConfig(World& world, EventBus& events)
            : m_world(world),
              m_events(events) {}
        
        template<Component T>
        void component();

        template<System T>
        Scheduler::SystemHandle system(
            std::string name,
            double frequency,
            T system
        );

        template<Spawner T>
        void entity_type(std::string name);

        template<typename T>
        Subscription subscribe(
            std::function<void(const T&)> callback
        );

    private:
        World& m_world;
        EventBus& m_events;
    };

    template<Component T>
    void PluginConfig::component() {
        m_world.register_component<T>();
    }

    template<System T>
    Scheduler::SystemHandle PluginConfig::system(
        std::string name,
        double frequency,
        T system
    ) {
        return m_world.add_system<T>(
            name,
            frequency,
            system
        );
    }

    template<Spawner T>
    void PluginConfig::entity_type(std::string name) {
        T spawner;
        SpawnConfig config;

        spawner.configure(config);

        m_world.register_spawner(name, config);
    }

    template<typename T>
    Subscription PluginConfig::subscribe(
        std::function<void(const T&)> callback
    ) {
        return m_events.subscribe(callback);
    }
}