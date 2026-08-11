#pragma once

#include <cstdint>
#include <type_traits>

#include "ecs/Entity.h"
#include "DeferredEntity.h"

namespace conductor {
    enum class EntityReferenceType: uint8_t {
        Entity,
        DeferredEntity
    };

    struct EntityReference {
        EntityReferenceType type;

        union {
            Entity entity;
            DeferredEntity deferred;
        };
        
        static constexpr EntityReference from_entity(
            Entity entity
        ) noexcept {
            EntityReference reference{};
            reference.type = EntityReferenceType::Entity;
            reference.entity = entity;
            return reference;
        }

        static constexpr EntityReference from_deferred(
            DeferredEntity deferred
        ) noexcept {
            EntityReference reference{};
            reference.type = EntityReferenceType::DeferredEntity;
            reference.deferred = deferred;
            return reference;
        }
    };

    static_assert(std::is_trivially_copyable_v<EntityReference>);
    static_assert(std::is_standard_layout_v<EntityReference>);
}