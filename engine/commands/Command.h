#pragma once

#include <cstdint>
#include <type_traits>

#include "ecs/Entity.h"
#include "DeferredEntity.h"
#include "EntityReference.h"
#include "ecs/ComponentType.h"

namespace conductor {
    enum class CommandType : uint8_t {
        CreateEntity,
        DestroyEntity,
        AddComponent,
        RemoveComponent
    };

    struct Command {
        CommandType type{};
        
        union {
            struct {
                DeferredEntity deferred;
            } m_create_entity;

            struct {
                Entity entity;
            } m_destroy_entity;

            struct {
                EntityReference entity;
                ComponentTypeId component_type;
                uint32_t data_offset;
                uint32_t data_size;
            } m_add_component;

            struct {
                EntityReference entity;
                ComponentTypeId component_type;
            } m_remove_component;
        };

        // Helper function to generate a create entity command
        static Command create_entity(DeferredEntity deferred) noexcept;

        // Helper function to generate a destroy entity command
        static Command destroy_entity(Entity entity) noexcept;

        // Helper function to generate an add component command
        static Command add_component(
            EntityReference entity, 
            ComponentTypeId component_type, 
            uint32_t data_offset, 
            uint32_t data_size
        ) noexcept;

        // Helper function to generate a remove component command
        static Command remove_component(
            EntityReference entity,
            ComponentTypeId component_type
        ) noexcept;        
    };

    static_assert(std::is_trivially_copyable_v<Command>);
    static_assert(std::is_standard_layout_v<Command>);
}