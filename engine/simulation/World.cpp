#include "World.h"

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

        Entity e = m_entities.create();
        return e;
    }

    void World::destroy_entity(Entity entity) {
        if(m_finalized) {
            throw std::logic_error(
                "Cannot destroy entities after world finalization"
            );
        }

        m_entities.destroy(entity);
    }

    void World::update(double delta) {
        m_clock.update(delta);
        m_scheduler.update(delta);
    }

    EntityManager& World::entities() {
        return m_entities;
    }

    const EntityManager& World::entities() const {
        return m_entities;
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