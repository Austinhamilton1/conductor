#pragma once

#include <cstdint>
#include <limits>
#include <vector>

#include "Entity.h"

namespace conductor {
    class EntityManager {
    public:
        Entity create();

        void destroy(Entity entity);

        bool alive(Entity entity) const noexcept;

        void reserve(size_t capacity);

        size_t size() const noexcept;
        
    private:
        std::vector<Entity::Generation> m_generations;
        std::vector<Entity::Index> m_free_indices;

        size_t m_alive_count = 0;
    };
}