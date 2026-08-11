#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include "Command.h"
#include "core/NonCopyable.h"
#include "ecs/EntityRegistry.h"
#include "ecs/ComponentType.h"

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
         * Queue addition of a component to an existing entity.
         */
        template<Component T>
        void add_component(
            Entity entity,
            const T& component
        );

        /*
         * Queue addition of a component to a deferred entity.
         */
        template<Component T>
        void add_component(
            DeferredEntity entity, 
            const T& component
        );

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
         * Commands are executed in insertion order.
         * 
         * The queue is cleared but the deferred entity resolutions are NOT 
         * cleared automatically. Call clear() when the synchronization phase
         * is complete.
         */
        void apply(EntityRegistry& registry);

        /*
         * Remove all commands and invalidate all DeferredEntity handles
         * associated with this batch.
         */
        void clear();

        bool empty() const noexcept;

        SizeType size() const noexcept;

        const Command *commands() const noexcept;

        const std::byte *data() const noexcept;

        SizeType data_size() const noexcept;

        /*
         * Reserve command storage ahead of time.
         */
        void reserve(SizeType capacity);

        /*
         * Reserve component data storage.
         */
        void reserve_data(SizeType capacity);

    private:
        void add_component(
            EntityReference entity,
            ComponentTypeId component_type,
            const void *data,
            SizeType size
        );

    private:
        std::vector<Command> m_commands;

        // Raw component dat associated with AddComponent commands
        std::vector<std::byte> m_data;

        // Maps DeferredEntity::Id -> resolved Entity
        std::vector<Entity> m_resolved_entities;
    };
}

template<conductor::Component T>
void conductor::CommandQueue::add_component(
    Entity entity,
    const T& component
) {
    add_component(
        EntityReference::from_entity(entity),
        component_type_id<T>(),
        &component,
        sizeof(T)
    );
}

template<conductor::Component T>
void conductor::CommandQueue::add_component(
    DeferredEntity entity,
    const T& component
) {
    add_component(
        EntityReference::from_deferred(entity),
        component_type_id<T>(),
        &component,
        sizeof(T)
    );
}