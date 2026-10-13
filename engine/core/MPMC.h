#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "NonCopyable.h"

namespace conductor {
    namespace detail {
        constexpr std::size_t default_capacity = 4096;
        constexpr std::size_t cache_line_size = 64;

        inline std::size_t validate_capacity(std::size_t capacity) {
            if(capacity == 0 || (capacity & (capacity - 1)) != 0) {
                throw std::invalid_argument(
                    "MPMC capacity must be nonzero power of two"
                );
            }

            if(capacity > std::numeric_limits<std::uint64_t>::max() / 2) {
                throw std::invalid_argument(
                    "MPMC capacity is too large"
                );
            }

            return capacity;
        }
    }

    template<typename T>
    class MPMC : public NonCopyable {
        static_assert(std::is_nothrow_constructible_v<T>,
            "MPMC requires nothrow move construction");

        static_assert(std::is_nothrow_destructible_v<T>,
            "MPMC requires nothrow destruction");

        // Helper struct representing a padded, uninitialized slot in memory
        struct Slot {
            std::atomic<uint64_t> sequence{0};

            // Aligned raw byte buffer instead of a fully formed T instance
            alignas(T) unsigned char storage[sizeof(T)];

            Slot() noexcept = default;
        };
    public:
        explicit MPMC(std::size_t capacity = detail::default_capacity)
            : m_capacity(detail::validate_capacity(capacity)),
              m_mask(m_capacity - 1),
              m_queue(std::make_unique<Slot[]>(m_capacity))
        {
            for(std::size_t i = 0; i < m_capacity; i++) {
                m_queue[i].sequence.store(
                    static_cast<std::uint64_t>(i),
                    std::memory_order_relaxed
                );
            }
        }

        ~MPMC() {
            // All producers and consumers must have stopped
            // before the queue is destroyed.
            while(try_pop().has_value()) {}
        }

        MPMC(MPMC&&) = delete;
        MPMC& operator=(MPMC&&) = delete;

        template<typename... Args>
            requires std::is_nothrow_constructible_v<T, Args...>
        bool try_emplace(Args&&... args) noexcept {
            Slot *slot;
            uint64_t pos = m_enqueue_pos.load(std::memory_order_relaxed);

            // Reserve new slot
            for(;;) {
                slot = &m_queue[pos & m_mask];

                const uint64_t seq = 
                    slot->sequence.load(std::memory_order_acquire);
                
                const auto dif = 
                    static_cast<int64_t>(seq) - 
                    static_cast<int64_t>(pos);

                if(dif == 0) {
                    // Try to claim this spot
                    if(m_enqueue_pos.compare_exchange_weak(
                        pos, pos + 1, 
                        std::memory_order_relaxed,
                        std::memory_order_relaxed)) {
                        break;
                    }
                } else if(dif < 0) {
                    return false; // Queue full
                } else {
                    pos = m_enqueue_pos.load(std::memory_order_relaxed);
                }
            }

            ::new (static_cast<void *>(&slot->storage))
                T(std::forward<Args>(args)...);

            // Publish fully constructed item.
            slot->sequence.store(pos + 1, std::memory_order_release);
            
            return true;
        }

        bool try_push(const T& value) noexcept 
            requires std::is_nothrow_copy_constructible_v<T>
        {
            return try_emplace(value);
        }

        bool try_push(T&& value) noexcept
            requires std::is_nothrow_copy_constructible_v<T>
        {
            return try_emplace(std::move(value));
        }

        [[nodiscard]]
        std::optional<T> try_pop() noexcept {
            Slot *slot;
            uint64_t pos = m_dequeue_pos.load(std::memory_order_relaxed);

            for(;;) {
                slot = &m_queue[pos & m_mask];
                
                const uint64_t seq = 
                    slot->sequence.load(std::memory_order_acquire);
                
                const auto dif = 
                    static_cast<int64_t>(seq) - 
                    static_cast<int64_t>(pos + 1);

                if(dif == 0) {
                    if(m_dequeue_pos.compare_exchange_weak(
                        pos, pos + 1, 
                        std::memory_order_relaxed)) {
                        break;
                    } 
                } else if(dif < 0) {
                    return std::nullopt; // Queue empty
                } else {
                    pos = m_dequeue_pos.load(std::memory_order_relaxed);
                }
            }

            T *item = std::launder(reinterpret_cast<T *>(slot->storage));
            
            // T is required to be nothrow move constructible.
            std::optional<T> result(std::in_place, std::move(*item));

            item->~T();

            // Release the slot for its next generation
            slot->sequence.store(pos + m_capacity, std::memory_order_release);

            return result;
        }

        // An estimate only. Not a synchronization primitive.
        [[nodiscard]]
        std::size_t size_approx() const noexcept {
            const auto enqueued = m_enqueue_pos.load(
                std::memory_order_relaxed
            );
            const auto dequeued = m_dequeue_pos.load(
                std::memory_order_relaxed
            );

            const auto count = enqueued - dequeued;
            
            return static_cast<std::size_t>(
                std::min<std::uint64_t>(count, m_capacity)
            );
        }

        [[nodiscard]]
        bool empty_approx() const noexcept {
            return size_approx() == 0;
        }

        [[nodiscard]]
        constexpr std::size_t capacity() const noexcept {
            return m_capacity;
        }

        [[nodiscard]]
        static constexpr bool is_lock_free() noexcept {
            return std::atomic<std::uint64_t>::is_always_lock_free;
        }

    private:
        std::size_t m_capacity;
        std::size_t m_mask;

        std::unique_ptr<Slot[]> m_queue;

        // Keep producer and consumer contention on separate
        // cache lines on conventional 64-byte cache-line systems.
        alignas(detail::cache_line_size)
        std::atomic<std::uint64_t> m_enqueue_pos{0};

        alignas(detail::cache_line_size)
        std::atomic<std::uint64_t> m_dequeue_pos{0};
    };
}