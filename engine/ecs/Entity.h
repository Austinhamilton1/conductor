#pragma once

#include <cstdint>
#include <limits>

namespace conductor {
    class Entity {
    public:
        using Index = uint32_t;
        using Generation = uint32_t;

        static constexpr Index INVALID_INDEX = std::numeric_limits<Index>::max();

        constexpr Entity() noexcept = default;

        constexpr Entity(Index index, Generation generation) noexcept
            : m_index(index),
              m_generation(generation) {}

        constexpr Index index() const noexcept {
            return m_index;
        }

        constexpr Generation generation() const noexcept {
            return m_generation;
        }

        constexpr bool valid() const noexcept {
            return m_index != INVALID_INDEX;
        }

        constexpr bool operator==(const Entity&) const noexcept = default;

    private:
        Index m_index = INVALID_INDEX;
        Generation m_generation = 0;
    };
}