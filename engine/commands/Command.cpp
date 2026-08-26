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
}