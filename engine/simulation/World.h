#pragma once

#include "core/NonCopyable.h"
#include "ecs/ComponentType.h"
#include "ecs/Entity.h"
#include "ecs/EntityManager.h"
#include "ecs/ComponentRegistry.h"
#include "ecs/ComponentStorageRegistry.h"
#include "commands/CommandQueue.h"

namespace conductor {
    class World : public NonCopyable {
    public:
        World();
        ~World();

        // Configuration
        template<Component T>
        void register_component();

        template<Component T>
        void create_storage();

        void add_system();

        void finalize();

        // Simulation
        Entity create_entity();
        void destroy_entity(Entity entity);

        void update(double delta);

        // Access
        EntityManager& entities();
        const EntityManager& entities() const;

        ComponentRegistry& components();
        const ComponentRegistry& components() const;

    private:
        EntityManager m_entities;
        ComponentRegistry m_components;
        ComponentStorageRegistry m_storages;
        CommandQueue m_commands;

        bool m_finalized{false};
    };
}