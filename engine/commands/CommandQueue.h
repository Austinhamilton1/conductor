#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

#include "Command.h"
#include "core/NonCopyable.h"
#include "ecs/EntityManager.h"
#include "ecs/ComponentType.h"
#include "ecs/ComponentRegistry.h"
#include "ecs/ComponentStorage.h"
#include "ecs/ComponentStorageRegistry.h"

namespace conductor {
    class CommandQueue : public NonCopyable {
    public:
        using SizeType = size_t;

        CommandQueue() = default;
        ~CommandQueue() = default;

        CommandQueue(CommandQueue&&) noexcept = default;
        CommandQueue& operator=(CommandQueue&&) noexcept = default;

        /*
         * Queue creation of a new entity.
         *
         * The entity does not exist until apply() is called.
         * 
         * @return A DeferredEntity that can be referenced by
         *         subsequent commands.
         */
        DeferredEntity create_entity();

        /*
         * Queue destruction of an existing entity.
         *
         * The entity remains alive until apply() is called.
         */
        void destroy_entity(Entity entity);

        /*
         * Queue destruction of a deferred entity.
         */
        void destroy_entity(DeferredEntity entity);

        /*
         * Queue addition of a component to an entity.
         *
         * The component value is copied into the command queue.
         */
        template<Component T>
        void add_component(
            Entity entity,
            const T& component
        ) {
            add_component(
                EntityReference::from_entity(entity),
                &component,
                ComponentRegistry::register_component<T>(),
                sizeof(T)
            );
        }

        /*
         * Queue addition of a component to a deferred entity.
         
         */
        template<Component T>
        void add_component(
            DeferredEntity entity,
            const T& component
        ) {
            add_component(
                EntityReference::from_deferred(entity),
                &component,
                ComponentRegistry::register_component<T>(),
                sizeof(T)
            );
        }

        /*
         * Queue addition of a component to an EntityReference.
         */
        template<Component T>
        void add_component(
            EntityReference entity,
            const T& component
        ) {
            add_component(
                entity,
                &component,
                ComponentRegistry::register_component<T>(),
                sizeof(T)
            );
        }

        /*
         * Queue removal of a component from an entity.
         */
        template<Component T>
        void remove_component(
            Entity entity
        ) {
            remove_component(
                EntityReference::from_entity(entity),
                ComponentRegistry::register_component<T>()
            );
        }

        /*
         * Queue removal of a component to a deferred entity.
         */
        template<Component T>
        void remove_component(
            DeferredEntity entity
        ) {
            remove_component(
                EntityReference::from_deferred(entity),
                ComponentRegistry::register_component<T>()
            );
        }

        /*
         * Resolve a DeferredEntity into the actual Entity created
         * during the last apply().
         * 
         * @throws std::out_of_range if the deferred entity is invalid
         *         or has not been resolved.
         */
        Entity resolve(DeferredEntity deferred) const;

        /*
         * Apply all queued commands to the entity registry.
         *
         * Entity commands are applied first.
         * Component commands are applied second.
         * 
         * The queue is cleared but the deferred entity resolutions are NOT 
         * cleared automatically. Call clear() when the synchronization phase
         * is complete.
         */
        void apply(
            EntityManager& manager,
            ComponentStorageRegistry& storages
        );

        /*
         * Remove all commands and invalidate all DeferredEntity handles
         * associated with this batch.
         */
        void clear();

        bool empty() const noexcept;

        SizeType size() const noexcept;

        const Command *commands() const noexcept;

        /*
         * Reserve command storage ahead of time.
         */
        void reserve(SizeType capacity);

    private:
        void add_component(
            EntityReference entity,
            const void *component,
            ComponentTypeId component_type,
            SizeType component_size
        );

        void remove_component(
            EntityReference entity,
            ComponentTypeId component
        );

        Entity resolve_reference(EntityReference reference) const;

        void apply_entity_command(
            const Command& command,
            EntityManager& manager
        );

        void apply_component_command(
            const Command& command,
            EntityManager& manager,
            ComponentStorageRegistry& storages
        );

    private:
        std::vector<Command> m_commands;

        // Maps DeferredEntity::Id -> resolved Entity.
        std::vector<Entity> m_resolved_entities;

        // Raw component data owned by the queue.
        std::vector<std::byte> m_component_data;
    };
}