#include "World.h"

namespace conductor {
    void SpawnConfig::initialize(World& world, Entity entity, SpawnContext& context) const {
        for(auto& initializer : m_initializers) {
            initializer(world, entity, context);
        }
    }
    
    void World::finalize() {
        m_storages.flush();
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

    void World::register_spawner(const std::string& name, SpawnConfig config) {
        if(m_finalized) {
            throw std::logic_error(
                "Cannot register spawners after world finalization"
            );
        }

        m_spawners[name] = config;
    }

    void World::spawn_entities(std::size_t count, const std::string& name) {
        if(m_finalized) {
            throw std::logic_error(
                "Cannot create entities after world finalization"
            );
        }

        if(m_spawners.find(name) == m_spawners.end()) {
            return;
        }

        SpawnConfig config = m_spawners[name];
        
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

    WorldContext World::get_context(ApplicationContext ctx) {
        return WorldContext{
            .random = ctx.random,
            .clock = ctx.clock,
            .events = ctx.events,
            .executor = ctx.executor,
            .queue = m_commands
        };
    }

    void World::update(double delta, ApplicationContext ctx) {
        m_scheduler.update(delta, get_context(ctx));
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
}