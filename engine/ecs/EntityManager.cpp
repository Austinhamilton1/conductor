#include "EntityManager.h"

namespace conductor {
    Entity EntityManager::create() {
        if(!m_free_indices.empty()) {
            const Entity::Index index = m_free_indices.back();
            m_free_indices.pop_back();

            ++m_alive_count;

            return Entity(index, m_generations[index]);
        }

        const auto index = static_cast<Entity::Index>(m_generations.size());

        m_generations.push_back(0);
        ++m_alive_count;

        return Entity(index, 0);
    }

    void EntityManager::destroy(Entity entity) {
        if(!alive(entity))
            return;

        const Entity::Index index = entity.index();

        ++m_generations[index];
        m_free_indices.push_back(index);

        --m_alive_count;
    }

    bool EntityManager::alive(Entity entity) const noexcept {
        if(!entity.valid())
            return false;

        if(entity.index() >= m_generations.size())
            return false;

        return m_generations[entity.index()] == entity.generation();
    }

    void EntityManager::reserve(size_t capacity) {
        m_generations.reserve(capacity);
    }

    size_t EntityManager::size() const noexcept {
        return m_alive_count;
    }
}