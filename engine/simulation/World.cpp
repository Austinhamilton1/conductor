#include "World.h"
#include "SpawnConfig.h"

namespace conductor {
    void World::finalize() {
        m_finalized = true;
    }

    Entity World::create_entity() {
        if(m_finalized) {
            throw std::logic_error(
                "Cannot create entities after world finalization"
            );
        }

        Entity e = m_entity_manager.create();
        m_entities.push_back(e);
        return e;
    }

    Entity World::get_entity(size_t i) {
        return m_entities[i];
    }

    void World::destroy_entity(Entity entity) {
        if(m_finalized) {
            throw std::logic_error(
                "Cannot destroy entities after world finalization"
            );
        }

        m_entity_manager.destroy(entity);
    }

    void World::spawn_entities(std::size_t count, const SpawnConfig& config) {
        if(m_finalized) {
            throw std::logic_error(
                "Cannot create entities after world finalization"
            );
        }
        
        for(std::size_t i = 0; i < count; i++) {
            Entity e = create_entity();

            SpawnContext context{
                .entity = e,
                .index = i,
                .random = m_rand
            };

            config.initialize(*this, e, context);
        }
    }

    void World::update(double delta) {
        m_clock.update(delta);
        m_scheduler.update(delta);
        m_commands.apply(m_entity_manager, m_storages);
    }

    EntityManager& World::entities() {
        return m_entity_manager;
    }

    const EntityManager& World::entities() const {
        return m_entity_manager;
    }

    ComponentRegistry& World::components() {
        return m_components;
    }

    const ComponentRegistry& World::components() const {
        return m_components;
    }

    ComponentStorageRegistry& World::storage() {
        return m_storages;
    }

    const ComponentStorageRegistry& World::storage() const {
        return m_storages;
    }

    SimulationClock& World::clock() {
        return m_clock;
    }

    const SimulationClock& World::clock() const {
        return m_clock;
    }
}