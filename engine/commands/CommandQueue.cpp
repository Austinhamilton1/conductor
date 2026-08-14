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

    void CommandQueue::add_component(
        EntityReference entity,
        ComponentTypeId component_type,
        const void *data,
        CommandQueue::SizeType size
    ) {
        if(data == nullptr && size != 0) {
            throw std::invalid_argument(
                "Component data cannot be null when size is non-zero"
            );
        }

        if(size > 
            static_cast<SizeType>(
                std::numeric_limits<uint32_t>::max()
            )) {
                throw std::length_error(
                    "Component data is too large for a command"
                );
            }

        if(m_data.size() >
            static_cast<SizeType>(
                std::numeric_limits<uint32_t>::max()
            ) - size) {
                throw std::length_error(
                    "CommandQueue data buffer is too large"
                );
            }

        const auto offset = 
            static_cast<uint32_t>(m_data.size());

        const auto data_size = 
            static_cast<uint32_t>(size);

        const auto *bytes =
            static_cast<const std::byte *>(data);

        m_data.insert(
            m_data.end(),
            bytes,
            bytes + size
        );

        m_commands.push_back(
            Command::add_component(
                entity,
                component_type,
                offset,
                data_size
            )
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

            case CommandType::AddComponent:
                // Not implemented yet
                break;

            case CommandType::RemoveComponent:
                // Not implemented yet
                break;
            }
        }

        m_commands.clear();
        m_data.clear();
    }

    void CommandQueue::clear() {
        m_commands.clear();
        m_data.clear();
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

    void CommandQueue::reserve_data(SizeType capacity) {
        m_data.reserve(capacity);
    }

    const Command *CommandQueue::commands() const noexcept {
        return m_commands.data();
    }

    const std::byte *CommandQueue::data() const noexcept {
        return m_data.data();
    }

    CommandQueue::SizeType CommandQueue::data_size() const noexcept {
        return m_data.size();
    }
}