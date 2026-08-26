#include "CommandQueue.h"

#include <limits>
#include <stdexcept>

namespace conductor {
    DeferredEntity CommandQueue::create_entity() {
        const auto id = 
            static_cast<DeferredEntity::Id>(
                m_resolved_entities.size()
            );

        m_resolved_entities.emplace_back();

        const DeferredEntity deferred{id};

        m_commands.push_back(
            Command::create_entity(deferred)
        );

        return deferred;
    }

    void CommandQueue::destroy_entity(Entity entity) {
        m_commands.push_back(
            Command::destroy_entity(entity)
        );
    }

    Entity CommandQueue::resolve(DeferredEntity deferred) const {
        if(!deferred.valid()) {
            throw std::out_of_range(
                "Attempted to resolve an invalid DeferredEntity"
            );
        }

        const auto id = deferred.id();

        if(id >= m_resolved_entities.size()) {
            throw std::out_of_range(
                "DeferredEntity does not belong to this CommandQueue"
            );
        }

        const Entity entity = 
            m_resolved_entities[id];

        if(!entity.valid()) {
            throw std::out_of_range(
                "DeferredEntity has not been resolved"
            );
        }

        return entity;
    }

    void CommandQueue::apply(EntityManager& manager) {
        for(const Command& command: m_commands) {
            switch (command.type) {
            case CommandType::CreateEntity: {
                const Entity entity = 
                    manager.create();

                const auto id = 
                    command.m_create_entity.deferred.id();

                if(id >= m_resolved_entities.size()) {
                    throw std::out_of_range(
                        "CreateEntity contains an invalid DeferredEntity"
                    );
                }

                m_resolved_entities[id] = entity;
                break;
            }
            
            case CommandType::DestroyEntity:
                manager.destroy(
                    command.m_destroy_entity.entity
                );
                break;
            }
        }

        m_commands.clear();
    }

    void CommandQueue::clear() {
        m_commands.clear();
        m_resolved_entities.clear();
    }

    bool CommandQueue::empty() const noexcept {
        return m_commands.empty();
    }

    CommandQueue::SizeType CommandQueue::size() const noexcept {
        return m_commands.size();
    }

    void CommandQueue::reserve(SizeType capacity) {
        m_commands.reserve(capacity);
    }

    const Command *CommandQueue::commands() const noexcept {
        return m_commands.data();
    }
}