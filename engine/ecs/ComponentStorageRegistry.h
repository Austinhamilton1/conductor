#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "ComponentRegistry.h"
#include "ComponentStorage.h"
#include "core/NonCopyable.h"

namespace conductor {
    class ComponentStorageRegistry : public NonCopyable {
    public:
        struct StorageEntry {
            void *storage = nullptr;
            void (*destroy)(void *) noexcept = nullptr;

            bool valid() const noexcept {
                return storage != nullptr;
            }
        };

        ComponentStorageRegistry() = default;

        ~ComponentStorageRegistry();

        template<Component T>
        ComponentStorage<T>& create() {
            const auto id =
                ComponentRegistry::register_component<T>();

            ensure_capacity(id);

            auto& entry = m_storages[id];

            if(entry.valid()) {
                throw std::logic_error(
                    "Component storage already exists"
                );
            }

            auto *storage =
                new ComponentStorage<T>();

            entry.storage = storage;
            entry.destroy = [](void *ptr) noexcept {
                delete static_cast<ComponentStorage<T>*>(ptr);
            };

            ++m_size;

            return *storage;
        }

        template<Component T>
        void destroy() {
            const auto id =
                ComponentRegistry::register_component<T>();

            destroy(id);
        }

        void destroy(ComponentTypeId id);

        template<Component T>
        bool contains() const noexcept {
            const auto id =
                ComponentRegistry::register_component<T>();

            return contains(id);
        }

        bool contains(ComponentTypeId id) const noexcept;

        template<Component T>
        ComponentStorage<T>& get() {
            const auto id =
                ComponentRegistry::register_component<T>();

            auto& entry = get_entry(id);

            return *static_cast<ComponentStorage<T>*>(
                entry.storage
            );
        }

        template<Component T>
        const ComponentStorage<T>& get() const {
            const auto id =
                ComponentRegistry::register_component<T>();

            const auto& entry = get_entry(id);

            return *static_cast<const ComponentStorage<T>*>(
                entry.storage
            );
        }

        StorageEntry& entry(ComponentTypeId id);

        const StorageEntry& entry(ComponentTypeId id) const;

        size_t size() const noexcept {
            return m_size;
        }

        size_t capacity() const noexcept {
            return m_storages.size();
        }

    private:
        void ensure_capacity(ComponentTypeId id);

        StorageEntry& get_entry(ComponentTypeId id);

        const StorageEntry& get_entry(ComponentTypeId id) const;

        std::vector<StorageEntry> m_storages;
        size_t m_size = 0;
    };
}