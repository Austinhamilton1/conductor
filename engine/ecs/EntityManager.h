#pragma once

#include <atomic>
#include <cstdint>
#include <limits>
#include <mutex>
#include <vector>

#include "Entity.h"

namespace conductor {
    class EntityManager {
    public:
        Entity create();

        void destroy(Entity entity);

        bool alive(Entity entity) noexcept;

        void reserve(size_t capacity);

        size_t size() const noexcept;
        
    private:
        std::mutex m_mutex;
        std::vector<Entity::Generation> m_generations;
        std::vector<Entity::Index> m_free_indices;

        std::atomic<std::size_t> m_alive_count{0};
    };
}