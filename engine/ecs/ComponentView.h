#pragma once

#include <cstddef>
#include <utility>

#include "Entity.h"

namespace conductor {
    template<typename Storage>
    class ComponentView {
    public:
        using storage_type = Storage;
        using component_type =
            typename Storage::component_type;

        using iterator = 
            decltype(std::declval<Storage&>().begin());

        using const_iterator =
            decltype(std::declval<const Storage&>().begin());

        ComponentView() noexcept = default;

        ComponentView(Storage *storage) noexcept
            : m_storage(storage) {}

        iterator begin() noexcept {
            return m_storage->begin();
        }

        iterator end() noexcept {
            return m_storage->end();
        }

        const_iterator begin() const noexcept {
            return m_storage->begin();
        }

        const_iterator end() const noexcept {
            return m_storage->end();
        }

        size_t size() const noexcept {
            return m_storage->size();
        }

        bool empty() const noexcept {
            return m_storage->empty();
        }

        bool contains(Entity entity) const noexcept {
            return m_storage->contains(entity);
        }

        component_type *get(Entity entity) noexcept {
            return m_storage->get(entity);
        }

        const component_type *get(Entity entity) const noexcept {
            return m_storage->get(entity);
        }

    private:
        Storage *m_storage = nullptr;
    };
}