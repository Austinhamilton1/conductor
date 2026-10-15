#include <catch2/catch_test_macros.hpp>

#include <thread>
#include <vector>

#include "events/EventQueue.h"
#include "TestEvent.h"

namespace {
    constexpr int events_per_thread = 100;
    
    int get_thread_count() {
        const unsigned int reported =
            std::thread::hardware_concurrency();

        // Keep tests repeatable and avoid creating excessive threads.
        return static_cast<int>(
            std::min(reported == 0 ? 4u : reported, 8u)
        );
    }

    class SeenItems {
    public:
        explicit SeenItems(int total)
            : m_total(total), 
              m_seen(static_cast<std::size_t>(total) + 1) 
        {
            for(auto& entry : m_seen) {
                entry.store(0, std::memory_order_relaxed);
            }
        }

        void record(int value) noexcept {
            if(value < 1 || value > m_total) {
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
            return m_seen[static_cast<std::size_t>(value)].load(std::memory_order_relaxed);
        }

        int total() const noexcept {
            return m_total;
        }

    private:
        int m_total;
        std::vector<std::atomic<int>> m_seen;
        std::atomic<int> m_consumed{0};
        std::atomic<int> m_invalid{0};
    };

    void verify_all_items_seen_once(const SeenItems& seen) {
        REQUIRE(seen.invalid() == 0);
        REQUIRE(seen.consumed() == seen.total());

        for(int value = 1; value <= seen.total(); value++) {
            INFO("Checking item " << value);
            REQUIRE(seen.occurrences(value) == 1);
        }
    }
}

TEST_CASE("EventQueue starts empty", "[events]") {
    conductor::EventQueue queue;

    REQUIRE(queue.empty());
}

TEST_CASE("Push adds event", "[events]") {
    conductor::EventQueue queue;

    queue.push(TestEvent{5});

    REQUIRE_FALSE(queue.empty());
}

TEST_CASE("Pop returns correct payload", "[events]") {
    conductor::EventQueue queue;

    queue.push(TestEvent{42});

    auto event = queue.pop();

    auto *payload = static_cast<TestEvent *>(event.payload);

    REQUIRE(payload->value == 42);

    event.destroy(event.payload);
}

TEST_CASE("Queue preserves FIFO order", "[events]") {
    conductor::EventQueue queue;

    queue.push(TestEvent{1});
    queue.push(TestEvent{2});
    queue.push(TestEvent{3});

    auto e1 = queue.pop();
    auto e2 = queue.pop();
    auto e3 = queue.pop();

    REQUIRE(static_cast<TestEvent *>(e1.payload)->value == 1);
    REQUIRE(static_cast<TestEvent *>(e2.payload)->value == 2);
    REQUIRE(static_cast<TestEvent *>(e3.payload)->value == 3);

    e1.destroy(e1.payload);
    e2.destroy(e2.payload);
    e3.destroy(e3.payload);
}

TEST_CASE("Clear removes all events", "[events]") {
    conductor::EventQueue queue;

    queue.push(TestEvent{1});
    queue.push(TestEvent{2});

    queue.clear();

    REQUIRE(queue.empty());
}

TEST_CASE("Multithreaded push works", "[events]") {
    conductor::EventQueue queue;

    int thread_count = get_thread_count();
    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    for(int i = 0; i < thread_count; i++) {
        threads.emplace_back([&, i]() {
            for(int j = 1; j <= events_per_thread; j++) {
                int value = i * events_per_thread + j;
                queue.push(TestEvent{value});
            }
        });
    }

    for(auto& thread : threads) {
        thread.join();
    }

    int total_events = thread_count * events_per_thread;

    SeenItems seen(total_events);
    while(!queue.empty()) {
        auto e = queue.pop();

        TestEvent *event = static_cast<TestEvent *>(e.payload);

        seen.record(event->value);
    }

    verify_all_items_seen_once(seen);
}