#include <chrono>
#include <cstdint>
#include <cstdio>

#include "ecs/ComponentStorage.h"
#include "ecs/Query.h"

namespace {
    struct Position {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };

    struct Velocity {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };

    using Clock = std::chrono::steady_clock;

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

    using Query = conductor::Query<conductor::Write<Position>, conductor::Read<Velocity>>;
}

int main() {
    constexpr size_t component_count = 100'000;
    constexpr size_t iterations = 100;

    conductor::ComponentStorage<Position> positions;
    conductor::ComponentStorage<Velocity> velocities;
    
    positions.reserve(component_count);
    velocities.reserve(component_count);

    for(size_t i = 0; i < component_count; i++) {
        positions.add(
            conductor::Entity(i, 0),
            Position{
                .x = static_cast<float>(i),
                .y = static_cast<float>(i * 2),
                .z = static_cast<float>(i * 3)
            }
        );

        velocities.add(
            conductor::Entity(i, 0),
            Velocity{
                .x = 0.01,
                .y = 0.14,
                .z = 0.0987
            } 
        );
    }

    auto query = Query::bind(positions.view(), velocities.view());

    benchmark(
        "serial_iteration",
        [&]() {
            query.iter([&](conductor::Entity& e, Position& pos, const Velocity& vel) {
                double x = pos.x;
                double y = pos.y;
                double z = pos.z;

                for(int i = 0; i < 100; ++i) {
                    x = std::sin(x) + std::cos(y);
                    y = std::sin(y) + std::cos(z);
                    z = std::sin(z) + std::cos(x);
                }

                pos.x = x + vel.x;
                pos.y = y + vel.y;
                pos.z = z + vel.z;
            });
        },
        iterations
    );

    tf::Executor executor;

    benchmark(
        "parallel_iteration",
        [&]() {
            query.par_iter(executor, [&](conductor::Entity& e, Position& pos, const Velocity& vel) {
                double x = pos.x;
                double y = pos.y;
                double z = pos.z;

                for(int i = 0; i < 100; ++i) {
                    x = std::sin(x) + std::cos(y);
                    y = std::sin(y) + std::cos(z);
                    z = std::sin(z) + std::cos(x);
                }

                pos.x = x + vel.x;
                pos.y = y + vel.y;
                pos.z = z + vel.z;
            });
        },
        iterations
    );
}