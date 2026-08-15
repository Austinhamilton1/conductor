#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <type_traits>
#include <cassert>
#include <iterator>

#include "ComponentType.h"
#include "Entity.h"
#include "core/NonCopyable.h"

namespace conductor {
    template<Component T, size_t ChunkSize = 4096>
    class ComponentStorage : public NonCopyable {
        static_assert(ChunkSize > 0);

    private:
        struct Chunk {
            std::unique_ptr<T[]> components;
            std::unique_ptr<Entity[]> entities;
            size_t size = 0;

            Chunk() 
                : components(std::make_unique<T[]>(ChunkSize)),
                  entities(std::make_unique<Entity[]>(ChunkSize)) {}
        };

        struct Location {
            uint32_t chunk = 0;
            uint32_t offset = 0;
            Entity::Generation generation = 0;
            bool present = false;
        };

    public:
        class Iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            Iterator(
                ComponentStorage *storage,
                size_t index
            ) noexcept
                : m_storage(storage),
                  m_index(index) {}

            reference operator*() const noexcept {
                return m_storage->component_at(m_index);
            }

            pointer operator->() const noexcept {
                return &m_storage->component_at(m_index);
            }

            Iterator& operator++() noexcept {
                ++m_index;
                return *this;
            }

            bool operator==(const Iterator& other) const noexcept {
                return m_storage == other.m_storage &&
                       m_index == other.m_index;
            }

            bool operator!=(const Iterator& other) const noexcept {
                return !(*this == other);
            }

        private:
            ComponentStorage *m_storage;
            size_t m_index;
        };

        class ConstIterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = const T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

            ConstIterator(
                const ComponentStorage* storage,
                size_t index
            ) noexcept
                : m_storage(storage),
                  m_index(index) {}

            reference operator*() const noexcept {
                return m_storage->component_at(m_index);
            }

            pointer operator->() const noexcept {
                return &m_storage->component_at(m_index);
            }

            ConstIterator& operator++() noexcept {
                ++m_index;
                return *this;
            }

            ConstIterator operator++(int) noexcept {
                ConstIterator copy = *this;
                ++(*this);
                 return copy;
            }

            bool operator==(const ConstIterator& other) const noexcept {
                return m_storage == other.m_storage &&
                       m_index == other.m_index;
            }

            bool operator!=(const ConstIterator& other) const noexcept {
                return !(*this == other);
            }

        private:
            const ComponentStorage* m_storage;
            size_t m_index;
        };

    public:
        ComponentStorage() = default;

        explicit ComponentStorage(size_t entity_capacity) {
            reserve(entity_capacity);
        }

        ComponentStorage(ComponentStorage&&) noexcept = default;
        ComponentStorage& operator=(ComponentStorage&&) noexcept = default;

        ~ComponentStorage() = default;

        void reserve(size_t entity_capacity) {
            m_locations.reserve(entity_capacity);
        }

        T& add(Entity entity, const T& component) {
            assert(entity.valid());

            ensure_location_capacity(entity.index());

            Location& location = m_locations[entity.index()];

            // A live component already exists.
            if(location.present) {
                assert(location.generation == entity.generation());

                return component_at(location);
            }

            if(m_chunks.empty() || m_chunks.back()->size == ChunkSize) {
                m_chunks.push_back(std::make_unique<Chunk>());
            }

            Chunk& chunk = *m_chunks.back();

            const size_t offset = chunk.size;

            chunk.components[offset] = component;
            chunk.entities[offset] = entity;

            ++chunk.size;

            location.chunk = static_cast<uint32_t>(m_chunks.size() - 1);
            location.offset = static_cast<uint32_t>(offset);
            location.generation = entity.generation();
            location.present = true;

            ++m_size;

            return chunk.components[offset];
        }

        void remove(Entity entity) {
            if(!contains(entity))
                return;

            m_pending_removals.push_back(entity);
        }

        void flush_removals() {
            for(const Entity entity : m_pending_removals) {
                remove_immediate(entity);
            }

            m_pending_removals.clear();
        }

        bool contains(Entity entity) const noexcept {
            if(!entity.valid())
                return false;

            const size_t index = entity.index();

            if(index >= m_locations.size())
                return false;

            const Location& location = m_locations[index];

            return location.present && location.generation == entity.generation();
        }

        T *get(Entity entity) noexcept {
            if(!contains(entity))
                return nullptr;

            return &component_at(
                m_locations[entity.index()]
            );
        }

        const T *get(Entity entity) const noexcept {
            if(!contains(entity))
                return nullptr;

            return &component_at(
                m_locations[entity.index()]
            );
        }

        size_t size() const noexcept {
            return m_size;
        }

        bool empty() const noexcept {
            return m_size == 0;
        }

        size_t chunk_count() const noexcept {
            return m_chunks.size();
        }

        Iterator begin() noexcept {
            return Iterator(this, 0);
        }

        Iterator end() noexcept {
            return Iterator(this, m_size);
        }

        ConstIterator begin() const noexcept {
            return ConstIterator(this, 0);
        }

        ConstIterator end() const noexcept {
            return ConstIterator(this, m_size);
        }

        ConstIterator cbegin() const noexcept {
            return begin();
        }

        ConstIterator cend() const noexcept {
            return end();
        }

        T *data(size_t index) noexcept {
            assert(index < m_size);
            return &component_at(index);
        }

        const T *data(size_t index) const noexcept {
            assert(index < m_size);
            return &component_at(index);
        }

    private:
        void ensure_location_capacity(Entity::Index index) {
            if(index >= m_locations.size()) {
                m_locations.resize(
                    static_cast<size_t>(index) + 1
                );
            }
        }

        T& component_at(size_t index) noexcept {
            const size_t chunk_index = index / ChunkSize;
            const size_t offset = index % ChunkSize;

            return m_chunks[chunk_index]->components[offset];
        }

        const T& component_at(size_t index) const noexcept {
            const size_t chunk_index = index / ChunkSize;
            const size_t offset = index % ChunkSize;

            return m_chunks[chunk_index]->components[offset];
        }

        T& component_at(const Location& location) noexcept {
            return m_chunks[location.chunk]->components[location.offset];
        }

        const T& component_at(const Location& location) const noexcept {
            return m_chunks[location.chunk]->components[location.offset];
        }

        void remove_immediate(Entity entity) {
            if (!contains(entity))
                return;

            Location& removed_location =
                m_locations[entity.index()];

            Chunk& removed_chunk =
                *m_chunks[removed_location.chunk];

            const size_t removed_offset =
                removed_location.offset;

            Chunk& last_chunk =
                *m_chunks.back();

            const size_t last_offset =
                last_chunk.size - 1;

            /*
             * If the removed component isn't the final
             * component in storage, move the final component
             * into the removed slot.
             */
            if (&removed_chunk != &last_chunk ||
                removed_offset != last_offset) {

                const Entity moved_entity =
                    last_chunk.entities[last_offset];

                removed_chunk.components[removed_offset] =
                    last_chunk.components[last_offset];

                removed_chunk.entities[removed_offset] =
                    moved_entity;

                Location& moved_location =
                    m_locations[moved_entity.index()];

                moved_location.chunk =
                    removed_location.chunk;

                moved_location.offset =
                    removed_location.offset;
            }

            --last_chunk.size;
            --m_size;

            removed_location.present = false;

            /*
             * The last chunk is now empty. Release it.
             */
            if (last_chunk.size == 0) {
                m_chunks.pop_back();
            }
        }

        private:
            /*
             * Component data is stored densely in fixed-size
             * chunks. There is no std::vector<T> here intentionally.
             */
            std::vector<std::unique_ptr<Chunk>> m_chunks;

            /*
             * Entity index -> physical component location.
             *
             * This array may be sparse with respect to entity IDs,
             * but the actually component data remains densely packed.
             */
            std::vector<Location> m_locations;

            /*
             * Structural removals are deferred until the end
             * of the simulation frame.
             */
            std::vector<Entity> m_pending_removals;

            // Number of components currently stored.
            size_t m_size = 0;
    };
}