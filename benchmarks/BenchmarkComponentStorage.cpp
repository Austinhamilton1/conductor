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

    using Storage64 = conductor::ComponentStorage<Position, 64>;
    using Storage256 = conductor::ComponentStorage<Position, 256>;
    using Storage1024 = conductor::ComponentStorage<Position, 1024>;
    using Storage4096 = conductor::ComponentStorage<Position, 4096>;
    using Storage16384 = conductor::ComponentStorage<Position, 16384>;
}

int main() {
    constexpr size_t component_count = 1'000'000;
    constexpr size_t iterations = 100;

    Storage64 s64;
    Storage256 s256;
    Storage1024 s1024;
    Storage4096 s4096;
    Storage16384 s16384;

    s64.reserve(component_count);
    s256.reserve(component_count);
    s1024.reserve(component_count);
    s4096.reserve(component_count);
    s16384.reserve(component_count);

    for(uint32_t i = 0; i < component_count; i++) {
        s64.add(
            conductor::Entity(i, 0),
            Position{
                .x = static_cast<float>(i),
                .y = static_cast<float>(i * 2),
                .z = static_cast<float>(i * 3)
            }
        );

        s256.add(
            conductor::Entity(i, 0),
            Position{
                .x = static_cast<float>(i),
                .y = static_cast<float>(i * 2),
                .z = static_cast<float>(i * 3)
            }
        );

        s1024.add(
            conductor::Entity(i, 0),
            Position{
                .x = static_cast<float>(i),
                .y = static_cast<float>(i * 2),
                .z = static_cast<float>(i * 3)
            }
        );

        s4096.add(
            conductor::Entity(i, 0),
            Position{
                .x = static_cast<float>(i),
                .y = static_cast<float>(i * 2),
                .z = static_cast<float>(i * 3)
            }
        );

        s16384.add(
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
        s64.size()
    );

    std::printf(
        "Chunks:     %zu\n\n",
        s64.chunk_count()
    );

    benchmark(
        "Range iteration",
        [&]() {
            float sum = 0.0f;

            for(const Position& position : s64) {
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

            for(size_t i = 0; i < s64.size(); i++) {
                sum += s64.data(i)->x;
            }

            g_sink = sum;
        },
        iterations
    );

    std::printf("\n");

    std::printf(
        "Components: %zu\n",
        s256.size()
    );

    std::printf(
        "Chunks:     %zu\n\n",
        s256.chunk_count()
    );

    benchmark(
        "Range iteration",
        [&]() {
            float sum = 0.0f;

            for(const Position& position : s256) {
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

            for(size_t i = 0; i < s256.size(); i++) {
                sum += s256.data(i)->x;
            }

            g_sink = sum;
        },
        iterations
    );

    std::printf("\n");

    std::printf(
        "Components: %zu\n",
        s1024.size()
    );

    std::printf(
        "Chunks:     %zu\n\n",
        s1024.chunk_count()
    );

    benchmark(
        "Range iteration",
        [&]() {
            float sum = 0.0f;

            for(const Position& position : s1024) {
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

            for(size_t i = 0; i < s1024.size(); i++) {
                sum += s1024.data(i)->x;
            }

            g_sink = sum;
        },
        iterations
    );

    std::printf("\n");

    std::printf(
        "Components: %zu\n",
        s4096.size()
    );

    std::printf(
        "Chunks:     %zu\n\n",
        s4096.chunk_count()
    );

    benchmark(
        "Range iteration",
        [&]() {
            float sum = 0.0f;

            for(const Position& position : s4096) {
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

            for(size_t i = 0; i < s4096.size(); i++) {
                sum += s4096.data(i)->x;
            }

            g_sink = sum;
        },
        iterations
    );

    std::printf("\n");

    std::printf(
        "Components: %zu\n",
        s16384.size()
    );

    std::printf(
        "Chunks:     %zu\n\n",
        s16384.chunk_count()
    );

    benchmark(
        "Range iteration",
        [&]() {
            float sum = 0.0f;

            for(const Position& position : s16384) {
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

            for(size_t i = 0; i < s16384.size(); i++) {
                sum += s16384.data(i)->x;
            }

            g_sink = sum;
        },
        iterations
    );
}