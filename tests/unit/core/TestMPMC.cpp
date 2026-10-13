
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

#include "core/MPMC.h"

namespace {
    constexpr int items_per_thread = 100;
    constexpr auto test_timeout = std::chrono::seconds(10);

    int get_thread_count() {
        const unsigned int reported =
            std::thread::hardware_concurrency();

        // Keep tests repeatable and avoid creating excessive threads.
        return static_cast<int>(
            std::min(reported == 0 ? 4u : reported, 8u));
    }

    std::size_t next_power_of_two(std::size_t n) {
        std::size_t capacity = 2;

        while (capacity < n) {
            capacity *= 2;
        }

        return capacity;
    }

    class SeenItems {
    public:
        explicit SeenItems(int total)
            : total_(total), m_seen(static_cast<std::size_t>(total) + 1) {
            for (auto& entry : m_seen) {
                entry.store(0, std::memory_order_relaxed);
            }
        }

        void record(int value) noexcept {
            if (value < 1 || value > total_) {
                m_invalid.fetch_add(1, std::memory_order_relaxed);
                return;
            }

            m_seen[static_cast<std::size_t>(value)].fetch_add(
                1, std::memory_order_relaxed);

            m_consumed.fetch_add(1, std::memory_order_relaxed);
        }

        int consumed() const noexcept {
            return m_consumed.load(std::memory_order_relaxed);
        }

        int invalid() const noexcept {
            return m_invalid.load(std::memory_order_relaxed);
        }

        int occurrences(int value) const noexcept {
            return m_seen[static_cast<std::size_t>(value)].load(
                std::memory_order_relaxed);
        }

        int total() const noexcept {
            return total_;
        }

    private:
        int total_;
        std::vector<std::atomic<int>> m_seen;
        std::atomic<int> m_consumed{0};
        std::atomic<int> m_invalid{0};
    };

    void verify_all_items_m_seenonce(const SeenItems& seen) {
        REQUIRE(seen.invalid() == 0);
        REQUIRE(seen.consumed() == seen.total());

        for (int value = 1; value <= seen.total(); ++value) {
            INFO("Checking item " << value);
            REQUIRE(seen.occurrences(value) == 1);
        }
    }
}

TEST_CASE("Every MPMC item is pushed exactly once", "[core]") {
    const int thread_count = get_thread_count();
    const int total_items = thread_count * items_per_thread;

    conductor::MPMC<int> queue(next_power_of_two(total_items));
    SeenItems seen(total_items);

    std::vector<std::thread> producers;
    producers.reserve(static_cast<std::size_t>(thread_count));

    for (int i = 0; i < thread_count; ++i) {
        producers.emplace_back([&queue, i]() {
            for (int j = 1; j <= items_per_thread; ++j) {
                const int value = i * items_per_thread + j;

                while (!queue.try_push(value)) {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& producer : producers) {
        producer.join();
    }

    while (auto value = queue.try_pop()) {
        seen.record(*value);
    }

    verify_all_items_m_seenonce(seen);
}

TEST_CASE("Every MPMC item is popped exactly once", "[core]") {
    const int thread_count = get_thread_count();
    const int total_items = thread_count * items_per_thread;

    conductor::MPMC<int> queue(next_power_of_two(total_items));
    SeenItems seen(total_items);

    // Fill the queue before starting multiple consumers.
    for (int value = 1; value <= total_items; ++value) {
        REQUIRE(queue.try_push(value));
    }

    std::vector<std::thread> consumers;
    consumers.reserve(static_cast<std::size_t>(thread_count));

    for (int i = 0; i < thread_count; ++i) {
        consumers.emplace_back([&queue, &seen]() {
            while (auto value = queue.try_pop()) {
                seen.record(*value);
            }
        });
    }

    for (auto& consumer : consumers) {
        consumer.join();
    }

    verify_all_items_m_seenonce(seen);
}

TEST_CASE("MPMC handles concurrent producer and consumer contention", "[core]") {
    const int thread_count = get_thread_count();
    const int total_items = thread_count * items_per_thread;

    // Deliberately small to exercise full/empty transitions and reuse.
    constexpr std::size_t capacity = 64;

    conductor::MPMC<int> queue(capacity);
    SeenItems seen(total_items);

    std::atomic<int> producers_done{0};
    std::atomic<bool> timed_out{false};

    const auto deadline =
        std::chrono::steady_clock::now() + test_timeout;

    auto check_deadline = [&]() {
        if (std::chrono::steady_clock::now() >= deadline) {
            timed_out.store(true, std::memory_order_relaxed);
            return true;
        }

        return timed_out.load(std::memory_order_relaxed);
    };

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    producers.reserve(static_cast<std::size_t>(thread_count));
    consumers.reserve(static_cast<std::size_t>(thread_count));

    for (int i = 0; i < thread_count; ++i) {
        producers.emplace_back([&, i]() {
            for (int j = 1; j <= items_per_thread; ++j) {
                if (check_deadline()) {
                    break;
                }

                const int value = i * items_per_thread + j;

                while (!queue.try_push(value)) {
                    if (check_deadline()) {
                        break;
                    }

                    std::this_thread::yield();
                }

                if (timed_out.load(std::memory_order_relaxed)) {
                    break;
                }
            }

            producers_done.fetch_add(1, std::memory_order_release);
        });
    }

    for (int i = 0; i < thread_count; ++i) {
        consumers.emplace_back([&]() {
            for (;;) {
                if (check_deadline()) {
                    break;
                }

                if (auto value = queue.try_pop()) {
                    seen.record(*value);
                    continue;
                }

                const bool all_producers_done =
                    producers_done.load(std::memory_order_acquire)
                    == thread_count;

                if (all_producers_done &&
                    seen.consumed() >= total_items) {
                    break;
                }

                std::this_thread::yield();
            }
        });
    }

    for (auto& producer : producers) {
        producer.join();
    }

    for (auto& consumer : consumers) {
        consumer.join();
    }

    REQUIRE_FALSE(timed_out.load(std::memory_order_relaxed));
    verify_all_items_m_seenonce(seen);
}
