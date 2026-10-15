
#include "EntityManager.h"

#include <limits>
#include <stdexcept>

namespace conductor {
    Entity EntityManager::create() {
        std::lock_guard lock(m_mutex);

        Entity::Index index;

        if (!m_free_indices.empty()) {
            index = m_free_indices.back();
            m_free_indices.pop_back();
        } else {
            if (m_generations.size() >=
                std::numeric_limits<Entity::Index>::max()) {
                throw std::length_error("Entity index capacity exhausted");
            }

            index = static_cast<Entity::Index>(
                m_generations.size());

            m_generations.push_back(0);
        }

        const auto generation = m_generations[index];

        m_alive_count.fetch_add(1, std::memory_order_relaxed);

        return Entity(index, generation);
    }

    void EntityManager::destroy(Entity entity) {
        if (!entity.valid()) {
            return;
        }

        std::lock_guard lock(m_mutex);

        const Entity::Index index = entity.index();

        if (index >= m_generations.size()) {
            return;
        }

        if (m_generations[index] != entity.generation()) {
            return;
        }

        // Invalidate the entity before making its index reusable.
        ++m_generations[index];
        m_free_indices.push_back(index);

        m_alive_count.fetch_sub(1, std::memory_order_relaxed);
    }

    bool EntityManager::alive(Entity entity) noexcept {
        if (!entity.valid()) {
            return false;
        }

        std::lock_guard lock(m_mutex);

        const Entity::Index index = entity.index();

        if (index >= m_generations.size()) {
            return false;
        }

        return m_generations[index] == entity.generation();
    }

    void EntityManager::reserve(std::size_t capacity) {
        std::lock_guard lock(m_mutex);
        m_generations.reserve(capacity);
        m_free_indices.reserve(capacity);
    }

    std::size_t EntityManager::size() const noexcept {
        return m_alive_count.load(std::memory_order_relaxed);
    }
}