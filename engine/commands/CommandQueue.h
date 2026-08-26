#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include "Command.h"
#include "core/NonCopyable.h"
#include "ecs/EntityManager.h"
#include "ecs/ComponentType.h"
#include "ecs/ComponentRegistry.h"
#include "ecs/ComponentStorage.h"

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
        void apply(EntityManager& manager);

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
        std::vector<Command> m_commands;

        // Maps DeferredEntity::Id -> resolved Entity
        std::vector<Entity> m_resolved_entities;
    };
}