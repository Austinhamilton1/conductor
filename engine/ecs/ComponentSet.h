#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "ComponentRegistry.h"

namespace conductor {
    class ComponentSet {
    public:
        ComponentSet() = default;

        template<Component T>
        void add();
        void add(ComponentTypeId id);

        template<Component T>
        bool contains() const noexcept;
        bool contains(ComponentTypeId id) const noexcept;
        bool contains(const ComponentSet& other) const noexcept;

        bool empty() const noexcept;
        size_t size() const noexcept;
        void clear();

        bool operator==(const ComponentSet&) const noexcept = default;

        private:
            static constexpr size_t BITS_PER_WORD = 64;

            static size_t word_index(ComponentTypeId id) noexcept;
            static uint64_t bit_mask(ComponentTypeId id) noexcept;

            std::vector<uint64_t> m_bits;
            size_t m_size = 0;
    };

    template<Component T>
    void ComponentSet::add() {
        add(ComponentRegistry::register_component<T>());
    }

    template<Component T>
    bool ComponentSet::contains() const noexcept {
        return contains(ComponentRegistry::register_component<T>());
    }
}