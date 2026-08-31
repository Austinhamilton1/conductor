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
        EntityReference entity
    ) noexcept {
        Command command{};
        
        command.type = CommandType::DestroyEntity;
        command.m_destroy_entity.entity = entity;
        
        return command;
    }

    Command Command::add_component(
        EntityReference entity,
        ComponentTypeId component,
        uint32_t data_offset,
        uint32_t data_size
    ) noexcept {
        Command command{};

        command.type = CommandType::AddComponent;
        command.m_add_component.entity = entity;
        command.m_add_component.component = component;
        command.m_add_component.data_offset = data_offset;
        command.m_add_component.data_size = data_size;

        return command;
    }

    Command Command::remove_component(
        EntityReference entity,
        ComponentTypeId component
    ) noexcept {
        Command command{};

        command.type = CommandType::RemoveComponent;
        command.m_remove_component.entity = entity;
        command.m_remove_component.component = component;

        return command;
    }
}