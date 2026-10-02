#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include <iostream>

#include "ComponentType.h"
#include "Entity.h"
#include "ComponentView.h"
#include "core/NonCopyable.h"
#include "core/Prefetch.h"


namespace conductor {
    namespace detail {
        inline constexpr size_t ComponentChunkSize = 4096;
    }

    template<Component T>
    class ComponentStorage : public NonCopyable {
    private:
        static constexpr size_t ChunkSize = detail::ComponentChunkSize;
        
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

            Iterator() noexcept = default;

            reference operator*() const noexcept {
                return *m_current;
            }

            pointer operator->() const noexcept {
                return m_current;
            }

            Entity entity() const noexcept {
                const Chunk& chunk =
                    m_storage->m_chunks[m_chunk];

                const size_t offset =
                    static_cast<size_t>(
                        m_current - chunk.components.get()
                    );

                return chunk.entities[offset];
            }

            Iterator& operator++() noexcept {
                ++m_current;

                /*
                 * Fast path:
                 *
                 * The vast majority of increments stay within
                 * the current chunk.
                 */
                if(m_current != m_chunk_end)
                    return *this;

                /*
                 * Slow path:
                 *
                 * We reached the end of the current chunk.
                 */
                ++m_chunk;

                if(m_chunk < m_storage->m_chunks.size()) {
                    load_chunk();
                }
                else {
                    /*
                     * End iterator.
                     */
                    m_current = nullptr;
                    m_chunk_end = nullptr;
                }

                return *this;
            }

            Iterator operator++(int) noexcept {
                Iterator copy = *this;
                ++(*this);
                return copy;
            }

            bool operator==(const Iterator& other) const noexcept {
                return m_storage == other.m_storage &&
                       m_current == other.m_current;
            }

            bool operator!=(const Iterator& other) const noexcept {
                return !(*this == other);
            }

        private:
            friend class ComponentStorage;

            Iterator(
                ComponentStorage* storage,
                size_t chunk
            ) noexcept
                : m_storage(storage),
                  m_chunk(chunk) {

                if(m_chunk < m_storage->m_chunks.size()) {
                    load_chunk();
                }
            }

            void load_chunk() noexcept {
                Chunk& chunk =
                    m_storage->m_chunks[m_chunk];

                m_current =
                    chunk.components.get();

                m_chunk_end =
                    m_current + chunk.size;

                /*
                 * Prefetch the following chunk while we're
                 * beginning to process the current one.
                 *
                 * This gives the CPU the entire current chunk
                 * as an opportunity to fetch the next chunk.
                 */
                const size_t next_chunk = m_chunk + 1;

                if(next_chunk < m_storage->m_chunks.size()) {
                    m_storage->prefetch_chunk(next_chunk);
                }
            }

            ComponentStorage* m_storage = nullptr;

            /*
             * Only consulted when crossing a chunk boundary.
             */
            size_t m_chunk = 0;

            /*
             * Hot-path state.
             *
             * operator*() is now simply:
             *
             *     return *m_current;
             */
            T* m_current = nullptr;
            T* m_chunk_end = nullptr;
        };

        class ConstIterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = const T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

            ConstIterator() noexcept = default;

            reference operator*() const noexcept {
                return *m_current;
            }

            pointer operator->() const noexcept {
                return m_current;
            }

            Entity entity() const noexcept {
                const Chunk& chunk =
                    m_storage->m_chunks[m_chunk];

                const size_t offset =
                    static_cast<size_t>(
                        m_current - chunk.components.get()
                    );

                return chunk.entities[offset];
            }

            ConstIterator& operator++() noexcept {
                ++m_current;

                /*
                 * Fast path:
                 *
                 * Stay within the current chunk.
                 */
                if(m_current != m_chunk_end)
                    return *this;

                /*
                 * Slow path:
                 *
                 * Move to the next chunk.
                 */
                ++m_chunk;

                if(m_chunk < m_storage->m_chunks.size()) {
                    load_chunk();
                }
                else {
                    m_current = nullptr;
                    m_chunk_end = nullptr;
                }

                return *this;
            }

            ConstIterator operator++(int) noexcept {
                ConstIterator copy = *this;
                ++(*this);
                return copy;
            }

            bool operator==(const ConstIterator& other) const noexcept {
                return m_storage == other.m_storage &&
                       m_current == other.m_current;
            }

            bool operator!=(const ConstIterator& other) const noexcept {
                return !(*this == other);
            }

        private:
            friend class ComponentStorage;

            ConstIterator(
                const ComponentStorage* storage,
                size_t chunk
            ) noexcept
                : m_storage(storage),
                  m_chunk(chunk) {

                if(m_chunk < m_storage->m_chunks.size()) {
                    load_chunk();
                }
            }

            void load_chunk() noexcept {
                const Chunk& chunk =
                    m_storage->m_chunks[m_chunk];

                m_current =
                    chunk.components.get();

                m_chunk_end =
                    m_current + chunk.size;

                const size_t next_chunk = m_chunk + 1;

                if(next_chunk < m_storage->m_chunks.size()) {
                    m_storage->prefetch_chunk(next_chunk);
                }
            }

            const ComponentStorage* m_storage = nullptr;

            size_t m_chunk = 0;

            const T* m_current = nullptr;
            const T* m_chunk_end = nullptr;
        };

    public:
        using component_type = T;

        ComponentStorage() = default;

        explicit ComponentStorage(size_t entity_capacity) {
            reserve(entity_capacity);
        }

        ComponentStorage(ComponentStorage&&) noexcept = default;

        ComponentStorage& operator=(
            ComponentStorage&&
        ) noexcept = default;

        ~ComponentStorage() = default;

        void reserve(size_t entity_capacity) {
            m_locations.reserve(entity_capacity);

            /*
             * Reserve enough chunk descriptors as well.
             *
             * This is only an optimization for construction and
             * doesn't affect the component memory layout.
             */
            const size_t chunk_count =
                (entity_capacity + ChunkSize - 1) /
                ChunkSize;

            m_chunks.reserve(chunk_count);
        }

        T& add(
            Entity entity,
            const T& component
        ) {
            assert(entity.valid());

            ensure_location_capacity(entity.index());

            Location& location =
                m_locations[entity.index()];

            /*
             * A live component already exists.
             */
            if(location.present) {
                assert(
                    location.generation ==
                    entity.generation()
                );

                return component_at(location);
            }

            /*
             * Allocate a new chunk when necessary.
             */
            if(
                m_chunks.empty() ||
                m_chunks.back().size == ChunkSize
            ) {
                m_chunks.emplace_back();
            }

            Chunk& chunk = m_chunks.back();

            const size_t offset = chunk.size;

            chunk.components[offset] = component;
            chunk.entities[offset] = entity;

            ++chunk.size;

            location.chunk =
                static_cast<uint32_t>(
                    m_chunks.size() - 1
                );

            location.offset =
                static_cast<uint32_t>(offset);

            location.generation =
                entity.generation();

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

            const Location& location =
                m_locations[index];

            return location.present &&
                   location.generation ==
                       entity.generation();
        }

        T* get(Entity entity) noexcept {
            if(!contains(entity))
                return nullptr;

            return &component_at(
                m_locations[entity.index()]
            );
        }

        const T* get(Entity entity) const noexcept {
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
            if(m_chunks.empty())
                return end();

            return Iterator(this, 0);
        }

        Iterator end() noexcept {
            return Iterator(
                this,
                m_chunks.size()
            );
        }

        ConstIterator begin() const noexcept {
            if(m_chunks.empty())
                return end();

            return ConstIterator(this, 0);
        }

        ConstIterator end() const noexcept {
            return ConstIterator(
                this,
                m_chunks.size()
            );
        }

        ConstIterator cbegin() const noexcept {
            return begin();
        }

        ConstIterator cend() const noexcept {
            return end();
        }

        ComponentView<ComponentStorage> view() noexcept {
            return ComponentView<ComponentStorage>(this);
        }

        ComponentView<const ComponentStorage> view() const noexcept {
            return ComponentView<const ComponentStorage>(this);
        }

        T* data(size_t index) noexcept {
            assert(index < m_size);

            return &component_at(index);
        }

        const T* data(size_t index) const noexcept {
            assert(index < m_size);

            return &component_at(index);
        }

        Entity* entity(size_t index) noexcept {
            assert(index < m_size);

            return &entity_at(index);
        }

        const Entity* entity(size_t index) const noexcept {
            assert(index < m_size);

            return &entity_at(index);
        }

    private:
        void ensure_location_capacity(
            Entity::Index index
        ) {
            if(index >= m_locations.size()) {
                m_locations.resize(
                    static_cast<size_t>(index) + 1
                );
            }
        }

        Entity& entity_at(size_t index) noexcept {
            const size_t chunk_index =
                index / ChunkSize;

            const size_t offset =
                index % ChunkSize;

            return m_chunks[chunk_index]
                .entities[offset];
        }

        const Entity& entity_at(size_t index) const noexcept {
            const size_t chunk_index =
                index / ChunkSize;

            const size_t offset =
                index % ChunkSize;

            return m_chunks[chunk_index]
                .entities[offset];
        }

        /*
         * Used for random/indexed access.
         *
         * Iteration never comes through this function.
         */
        T& component_at(size_t index) noexcept {
            const size_t chunk_index =
                index / ChunkSize;

            const size_t offset =
                index % ChunkSize;

            return m_chunks[chunk_index]
                .components[offset];
        }

        const T& component_at(
            size_t index
        ) const noexcept {
            const size_t chunk_index =
                index / ChunkSize;

            const size_t offset =
                index % ChunkSize;

            return m_chunks[chunk_index]
                .components[offset];
        }

        T& component_at(
            const Location& location
        ) noexcept {
            return m_chunks[location.chunk]
                .components[location.offset];
        }

        const T& component_at(
            const Location& location
        ) const noexcept {
            return m_chunks[location.chunk]
                .components[location.offset];
        }

        void remove_immediate(Entity entity) {
            if(!contains(entity))
                return;

            Location& removed_location =
                m_locations[entity.index()];

            Chunk& removed_chunk =
                m_chunks[removed_location.chunk];

            const size_t removed_offset =
                removed_location.offset;

            Chunk& last_chunk =
                m_chunks.back();

            const size_t last_offset =
                last_chunk.size - 1;

            /*
             * If the removed component isn't the final
             * component in storage, move the final component
             * into the removed slot.
             */
            if(
                &removed_chunk != &last_chunk ||
                removed_offset != last_offset
            ) {
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
             * Release the final chunk if it is empty.
             */
            if(last_chunk.size == 0) {
                m_chunks.pop_back();
            }
        }

        void prefetch_chunk(
            size_t chunk_index
        ) const noexcept {
            if(chunk_index >= m_chunks.size())
                return;

            const Chunk& chunk =
                m_chunks[chunk_index];

            CONDUCTOR_PREFETCH_READ(
                chunk.components.get()
            );
        }

    private:
        /*
         * Chunk descriptors are now contiguous.
         *
         * Component and entity arrays remain separately allocated
         * fixed-size slabs.
         */
        std::vector<Chunk> m_chunks;

        /*
         * Entity index -> physical component location.
         *
         * This may be sparse with respect to entity IDs, while
         * component data itself remains densely packed.
         */
        std::vector<Location> m_locations;

        /*
         * Structural removals are deferred until the end
         * of the simulation frame.
         */
        std::vector<Entity> m_pending_removals;

        /*
         * Number of components currently stored.
         */
        size_t m_size = 0;
    };
}