#include <chrono>
#include <cstdint>
#include <cstdio>

#include "ecs/ComponentStorage.h"

namespace {
    struct Position {
        float x;
        float y;
        float z;
    };

    using Clock = std::chrono::steady_clock;

    /* 
     * Prevent the compiler from optimizing the benchmark
     * result away.
     */
    volatile float g_sink = 0.0f;

    template<typename Function>
    double benchmark(
        const char *name,
        Function&& function,
        size_t iterations
    ) {
        // Warmup
        function();

        const auto start = Clock::now();

        for(size_t i = 0; i < iterations; i++) {
            function();
        }

        const auto end = Clock::now();

        const auto elapsed =
            std::chrono::duration<double>(
                end - start
            ).count();

        const double total_components =
            static_cast<double>(iterations);

        std::printf(
            "%-30s %10.3f ms\n",
            name,
            elapsed * 1000.0
        );

        return elapsed;
    }
    
    using Storage = conductor::ComponentStorage<Position>;
}

int main() {
    constexpr size_t component_count = 1'000'000;
    constexpr size_t iterations = 100;

    Storage storage;

    storage.reserve(component_count);

    for(uint32_t i = 0; i < component_count; i++) {
        storage.add(
            conductor::Entity(i, 0),
            Position{
                .x = static_cast<float>(i),
                .y = static_cast<float>(i * 2),
                .z = static_cast<float>(i * 3)
            }
        );
    }

    std::printf(
        "Components: %zu\n",
        storage.size()
    );

    std::printf(
        "Chunks:     %zu\n\n",
        storage.chunk_count()
    );

    benchmark(
        "Range iteration",
        [&]() {
            float sum = 0.0f;

            for(const Position& position : storage) {
                sum += position.x;
            }

            g_sink = sum;
        },
        iterations
    );

    benchmark(
        "Indexed iteration",
        [&]() {
            float sum = 0.0f;

            for(size_t i = 0; i < storage.size(); i++) {
                sum += storage.data(i)->x;
            }

            g_sink = sum;
        },
        iterations
    );
}