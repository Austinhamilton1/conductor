#include "CommandQueue.h"

#include <limits>
#include <stdexcept>

namespace conductor {
    DeferredEntity CommandQueue::create_entity() {
        const auto id = 
            static_cast<DeferredEntity::Id>(
                m_resolved_entities.size()
            );

        if(id == DeferredEntity::INVALID_ID) {
            throw std::length_error(
                "CommandQueue has exhausted DeferredEntity IDs"
            );
        }

        m_resolved_entities.emplace_back();

        const DeferredEntity deferred{id};

        m_commands.push_back(
            Command::create_entity(deferred)
        );

        return deferred;
    }

    void CommandQueue::destroy_entity(Entity entity) {
        m_commands.push_back(
            Command::destroy_entity(
                EntityReference::from_entity(entity)
            )
        );
    }

    void CommandQueue::destroy_entity(DeferredEntity entity) {
        m_commands.push_back(
            Command::destroy_entity(
                EntityReference::from_deferred(entity)
            )
        );
    }

    void CommandQueue::add_component(
        EntityReference entity,
        const void *component,
        ComponentTypeId component_type,
        SizeType component_size
    ) {
        if(component == nullptr) {
            throw std::invalid_argument(
                "Cannot queue a null component"
            );
        }

        if(component_size >
            static_cast<SizeType>(
                std::numeric_limits<uint32_t>::max()
            )) {
                throw std::length_error(
                    "Component data exceeds Command size limit"
                );
            }

        const auto offset =
            m_component_data.size();

        const auto new_size =
            offset + component_size;

        if(new_size < offset) {
            throw std::length_error(
                "Command component data size overflow"
            );
        }

        m_component_data.resize(new_size);

        std::memcpy(
            m_component_data.data() + offset,
            component,
            component_size
        );

        m_commands.push_back(
            Command::add_component(
                entity,
                component_type,
                static_cast<uint32_t>(offset),
                static_cast<uint32_t>(component_size)
            )
        );
    }

    void CommandQueue::remove_component(
        EntityReference entity,
        ComponentTypeId component
    ) {
        m_commands.push_back(
            Command::remove_component(
                entity,
                component
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

    Entity CommandQueue::resolve_reference(EntityReference reference) const {
        switch (reference.type) {
        case EntityReferenceType::Entity:
            if(!reference.entity.valid()) {
                throw std::out_of_range(
                    "Command contains an invalid Entity"
                );
            }

            return reference.entity;
        
        case EntityReferenceType::DeferredEntity:
            return resolve(reference.deferred);
        }
        
        throw std::logic_error(
            "Invalid EntityReference type"
        );
    }

    void CommandQueue::apply_entity_command(
        const Command& command,
        EntityManager& manager,
        ComponentStorageRegistry& storages
    ) {
        switch(command.type) {
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

        case CommandType::DestroyEntity: {
            const Entity entity =
                resolve_reference(
                    command.m_destroy_entity.entity
                );

            manager.destroy(entity);
            storages.remove_entity(entity);
            break;
        }

        default:
            break;
        }
    }

    void CommandQueue::apply_component_command(
        const Command& command,
        EntityManager& manager,
        ComponentStorageRegistry& storages
    ) {
        switch(command.type) {
        case CommandType::AddComponent: {
            const Entity entity =
                resolve_reference(
                    command.m_add_component.entity
                );

            if(!manager.alive(entity)) {
                throw std::logic_error(
                    "Cannot add a component to a dead entity"
                );
            }

            const auto component =
                command.m_add_component.component;

            const auto& info =
                ComponentRegistry::info(component);

            const auto data_size =
                command.m_add_component.data_size;

            if(data_size != info.size) {
                throw std::logic_error(
                    "AddComponent data size does not match component type"
                );
            }

            const auto offset =
                static_cast<SizeType>(
                    command.m_add_component.data_offset
                );

            if(offset > m_component_data.size() ||
                data_size >
                m_component_data.size() - offset) {
                    throw std::out_of_range(
                        "AddComponent contains invalid component data"
                    );
                }

            auto& entry =
                storages.entry(component);

            if(entry.add == nullptr) {
                throw std::logic_error(
                    "Component storage does not support add"
                );
            }

            entry.add(
                entry.storage,
                entity,
                m_component_data.data() + offset
            );

            break;
        }

        case CommandType::RemoveComponent: {
            const auto entity =
                resolve_reference(
                    command.m_remove_component.entity
                );

            if(!manager.alive(entity)) {
                throw std::logic_error(
                    "Cannot remove a component from a dead entity"
                );
            }

            auto& entry =
                storages.entry(
                    command.m_remove_component.component
                );

            if(entry.remove == nullptr) {
                throw std::logic_error(
                    "Component storage does not support remove"
                );
            }

            entry.remove(
                entry.storage,
                entity
            );

            break;
        }

        default:
            break;
        }
    }

    void CommandQueue::apply(
        EntityManager& manager,
        ComponentStorageRegistry& storages
    ) {
        /*
         * Phase 1:
         * Apply every entity command.
         * 
         * There is intentionally no ordering contract between entity
         * commands beyond this phase boundary.
         */
        for(const Command& command : m_commands) {
            switch(command.type) {
            case CommandType::CreateEntity:
            case CommandType::DestroyEntity:
                apply_entity_command(
                    command,
                    manager,
                    storages
                );
                break;

            default:
                break;
            }
        }

        /*
         * Phase 2:
         * Apply every component command.
         * 
         * All DeferredEntity references are now resolvable because the
         * entire entity phase has completed.
         */
        for(const Command& command : m_commands) {
            switch (command.type) {
            case CommandType::AddComponent:
            case CommandType::RemoveComponent:
                apply_component_command(
                    command,
                    manager,
                    storages
                );
                break;
            
            default:
                break;
            }
        }

        storages.flush();
        m_commands.clear();
        m_component_data.clear();
    }

    void CommandQueue::clear() {
        m_commands.clear();
        m_resolved_entities.clear();
        m_component_data.clear();
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