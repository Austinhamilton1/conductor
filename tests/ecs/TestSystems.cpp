#include <cstddef>
#include <catch2/catch_test_macros.hpp>

#include "commands/CommandQueue.h"
#include "core/SimulationClock.h"
#include "ecs/Query.h"
#include "ecs/SystemContext.h"
#include "ecs/SystemExecutor.h"
#include "ecs/ComponentStorage.h"

namespace {
    struct Position {
        float x;
        float y;
        float z;
    };

    struct Velocity {
        float x;
        float y;
        float z;
    };

    struct MovementSystem {
        using Query = conductor::Query<
            conductor::Read<Velocity>,
            conductor::Write<Position>
        >;

        inline static int executions = 0;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>& ctx
        ) {
            ++executions;

            for(auto [velocity, position] :
                ctx.query())
            {
                position.x += velocity.x *
                              ctx.clock().delta_seconds();

                position.y += velocity.y *
                              ctx.clock().delta_seconds();

                position.z += velocity.z *
                              ctx.clock().delta_seconds();
            }
        }

        static void reset() {
            executions = 0;
        }
    };
}

using namespace conductor;

TEST_CASE("SystemExecutor provides functional query", "[ecs]") {
    MovementSystem::reset();

    ComponentStorage<Velocity> velocities;
    ComponentStorage<Position> positions;

    const Entity entity{
        0,
        0
    };

    velocities.add(
        entity,
        Velocity{
            .x = 10.0f,
            .y = 20.0f,
            .z = 30.0f
        }
    );

    positions.add(
        entity,
        Position{
            .x = 1.0f,
            .y = 2.0f,
            .z = 3.0f
        }
    );

    auto query =
        MovementSystem::Query::bind_storage(
            velocities,
            positions
        );

    CommandQueue commands;
    SimulationClock clock;

    SystemExecutor<
        MovementSystem,
        decltype(query)
    > executor(
        MovementSystem{},
        std::move(query)
    );

    clock.update(0.5);

    executor.execute(
        commands,
        clock
    );

    REQUIRE(MovementSystem::executions == 1);

    auto* result =
        positions.get(entity);

    REQUIRE(result != nullptr);

    REQUIRE(result->x == 6.0f);
    REQUIRE(result->y == 12.0f);
    REQUIRE(result->z == 18.0f);
}