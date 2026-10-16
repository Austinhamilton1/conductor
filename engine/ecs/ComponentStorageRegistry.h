#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
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
            void (*add)(
                void *,
                Entity,
                const void *
            ) = nullptr;
            void (*remove)(
                void *,
                Entity
            ) = nullptr;
            void (*flush)(void *) = nullptr;

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

            entry.add = [](
                void *ptr,
                Entity entity,
                const void *data
            ) {
                auto& typed_storage =
                    *static_cast<ComponentStorage<T>*>(ptr);

                T component{};

                std::memcpy(
                    &component,
                    data,
                    sizeof(T)
                );

                typed_storage.add(
                    entity,
                    component
                );
            };

            entry.remove = [](
                void *ptr,
                Entity entity
            ) {
                auto& typed_storage = 
                    *static_cast<ComponentStorage<T>*>(ptr);

                typed_storage.remove(entity);
            };

            entry.flush = [](void *ptr) {
                auto& typed_storage =
                    *static_cast<ComponentStorage<T>*>(ptr);

                typed_storage.flush_additions();
                typed_storage.flush_removals();
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

        void remove_entity(Entity entity);

        /*
         * Flush should be called after applying all Component
         * commands. Flush is not thread safe, so it should only
         * be called once by a single thread.
         */
        void flush();

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