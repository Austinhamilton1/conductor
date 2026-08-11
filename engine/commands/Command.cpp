#include "Command.h"

namespace conductor {
    Command Command::create_entity(
        DeferredEntity deferred
    ) noexcept {
        Command command{};
        
        command.type = CommandType::CreateEntity,
        command.m_create_entity.deferred = deferred;
        
        return command;
    }

    Command Command::destroy_entity(
        Entity entity
    ) noexcept {
        Command command{};
        
        command.type = CommandType::DestroyEntity;
        command.m_destroy_entity.entity = entity;
        
        return command;
    }

    Command Command::add_component(
        EntityReference entity,
        ComponentTypeId component_type,
        uint32_t data_offset,
        uint32_t data_size
    ) noexcept {
        Command command{};

        command.type = CommandType::AddComponent;
        command.m_add_component.entity = entity;
        command.m_add_component.component_type = component_type;
        command.m_add_component.data_offset = data_offset;
        command.m_add_component.data_size = data_size;

        return command;
    }

    Command Command::remove_component(
        EntityReference entity,
        ComponentTypeId component_type
    ) noexcept {
        Command command{};

        command.type = CommandType::RemoveComponent;
        command.m_remove_component.entity = entity;
        command.m_remove_component.component_type = component_type;

        return command;
    }
}