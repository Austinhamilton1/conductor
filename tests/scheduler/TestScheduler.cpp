#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <stdexcept>
#include <utility>

#include "scheduler/Scheduler.h"
#include "commands/CommandQueue.h"
#include "core/SimulationClock.h"

namespace {
    struct Position {
        float x;
        float y;
        float z;
    };

    struct ExecutionSystem {
        using Query = conductor::Query<
            conductor::Read<Position>
        >;

        inline static int executions = 0;
        inline static double last_delta = 0.0;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>& ctx
        ) {
            ++executions;

            last_delta =
                ctx.clock().delta_seconds();
        }

        static void reset() {
            executions = 0;
            last_delta = 0.0;
        }
    };
}

using namespace conductor;

// ============================================================
// Scheduling
// ============================================================

TEST_CASE("Scheduler creates a valid system handle") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "Movement",
        60.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(system.valid());
    REQUIRE(system.id() != 0);
    REQUIRE(system.generation() == 0);
    REQUIRE(system.state() == SystemState::Waiting);
    REQUIRE_FALSE(system.paused());
}


TEST_CASE("Scheduler assigns unique system IDs") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto first = scheduler.schedule(
        "First",
        60.0,
        ExecutionSystem{},
        query
    );

    auto second = scheduler.schedule(
        "Second",
        60.0,
        ExecutionSystem{},
        query
    );

    auto third = scheduler.schedule(
        "Third",
        60.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(first.id() != second.id());
    REQUIRE(first.id() != third.id());
    REQUIRE(second.id() != third.id());
}


TEST_CASE("Scheduler rejects zero frequency") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    REQUIRE_THROWS_AS(
        scheduler.schedule(
            "Invalid",
            0.0,
            ExecutionSystem{},
            query
        ),
        std::invalid_argument
    );
}


TEST_CASE("Scheduler rejects negative frequency") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    REQUIRE_THROWS_AS(
        scheduler.schedule(
            "Invalid",
            -60.0,
            ExecutionSystem{},
            query
        ),
        std::invalid_argument
    );
}


// ============================================================
// Timing
// ============================================================

TEST_CASE("System does not execute before its interval") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        10.0,
        ExecutionSystem{},
        query
    );

    // Interval = 0.1 seconds.
    scheduler.update(0.05);

    REQUIRE(system.generation() == 0);
    REQUIRE(system.state() == SystemState::Waiting);

    scheduler.update(0.049);

    REQUIRE(system.generation() == 0);
}


TEST_CASE("System executes when its interval is reached") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        10.0,
        ExecutionSystem{},
        query
    );

    // Interval = 0.1 seconds.
    scheduler.update(0.1);

    REQUIRE(system.generation() == 1);
    REQUIRE(system.state() == SystemState::Waiting);
}


TEST_CASE("System accumulates delta across updates") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        10.0,
        ExecutionSystem{},
        query
    );

    scheduler.update(0.03);

    REQUIRE(system.generation() == 0);

    scheduler.update(0.03);

    REQUIRE(system.generation() == 0);

    scheduler.update(0.04);

    REQUIRE(system.generation() == 1);
}


TEST_CASE("System can execute multiple times from accumulated delta") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        10.0,
        ExecutionSystem{},
        query
    );

    // 0.35 seconds at 10 Hz means three executions
    // with 0.05 seconds remaining.
    scheduler.update(0.35);

    REQUIRE(system.generation() == 3);
}


TEST_CASE("System retains fractional accumulated time") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        10.0,
        ExecutionSystem{},
        query
    );

    scheduler.update(0.35);

    REQUIRE(system.generation() == 3);

    scheduler.update(0.05);

    REQUIRE(system.generation() == 4);
}


TEST_CASE("System frequency can be changed") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        10.0,
        ExecutionSystem{},
        query
    );

    system.set_frequency(20.0);

    scheduler.update(0.05);

    REQUIRE(system.generation() == 1);
}


TEST_CASE("Invalid frequency change throws") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        60.0,
        ExecutionSystem{},
        query
    );

    REQUIRE_THROWS_AS(
        system.set_frequency(0.0),
        std::invalid_argument
    );

    REQUIRE_THROWS_AS(
        system.set_frequency(-10.0),
        std::invalid_argument
    );
}


// ============================================================
// Pause / Resume
// ============================================================

TEST_CASE("Paused system does not accumulate time") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        10.0,
        ExecutionSystem{},
        query
    );

    system.pause();

    REQUIRE(system.paused());

    scheduler.update(1.0);

    REQUIRE(system.generation() == 0);
}


TEST_CASE("Resuming system allows scheduling to continue") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        10.0,
        ExecutionSystem{},
        query
    );

    system.pause();

    scheduler.update(1.0);

    REQUIRE(system.generation() == 0);

    system.resume();

    REQUIRE_FALSE(system.paused());

    scheduler.update(0.1);

    REQUIRE(system.generation() == 1);
}


TEST_CASE("Pausing preserves accumulated time") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        10.0,
        ExecutionSystem{},
        query
    );

    scheduler.update(0.05);

    system.pause();

    scheduler.update(1.0);

    REQUIRE(system.generation() == 0);

    system.resume();

    scheduler.update(0.05);

    REQUIRE(system.generation() == 1);
}


// ============================================================
// Dependencies
// ============================================================

TEST_CASE("System can depend on another system") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto parent = scheduler.schedule(
        "Physics",
        60.0,
        ExecutionSystem{},
        query
    );

    auto child = scheduler.schedule(
        "Rendering",
        60.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        child.depends_on(parent) ==
        DependencyResult::Success
    );
}


TEST_CASE("System cannot depend on itself") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        60.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        system.depends_on(system) ==
        DependencyResult::SelfDependency
    );
}


TEST_CASE("Duplicate dependency is rejected") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto parent = scheduler.schedule(
        "Parent",
        60.0,
        ExecutionSystem{},
        query
    );

    auto child = scheduler.schedule(
        "Child",
        60.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        child.depends_on(parent) ==
        DependencyResult::Success
    );

    REQUIRE(
        child.depends_on(parent) ==
        DependencyResult::DuplicateDependency
    );
}


TEST_CASE("Dependency between different schedulers is rejected") {
    SimulationClock first_clock;
    CommandQueue first_queue;
    Scheduler first(first_queue, first_clock);

    SimulationClock second_clock;
    CommandQueue second_queue;
    Scheduler second(second_queue, second_clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto parent = first.schedule(
        "Parent",
        60.0,
        ExecutionSystem{},
        query
    );

    auto child = second.schedule(
        "Child",
        60.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        child.depends_on(parent) ==
        DependencyResult::InvalidParent
    );
}


// ============================================================
// Dependency ordering
// ============================================================

TEST_CASE("Child waits for parent generation") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto parent = scheduler.schedule(
        "Parent",
        10.0,
        ExecutionSystem{},
        query
    );

    auto child = scheduler.schedule(
        "Child",
        10.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        child.depends_on(parent) ==
        DependencyResult::Success
    );

    REQUIRE(parent.generation() == 0);
    REQUIRE(child.generation() == 0);

    scheduler.update(0.1);

    REQUIRE(parent.generation() == 1);
    REQUIRE(child.generation() == 1);
}


TEST_CASE("Child cannot execute twice without a new parent generation") { 
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto parent = scheduler.schedule(
        "Parent",
        10.0,
        ExecutionSystem{},
        query
    );

    auto child = scheduler.schedule(
        "Child",
        10.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        child.depends_on(parent) ==
        DependencyResult::Success
    );

    scheduler.update(0.1);

    REQUIRE(parent.generation() == 1);
    REQUIRE(child.generation() == 1);

    // Both are now waiting for another interval.
    scheduler.update(0.1);

    REQUIRE(parent.generation() == 2);
    REQUIRE(child.generation() == 2);
}


TEST_CASE("Dependency chain executes in generation order") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto a = scheduler.schedule(
        "A",
        10.0,
        ExecutionSystem{},
        query
    );

    auto b = scheduler.schedule(
        "B",
        10.0,
        ExecutionSystem{},
        query
    );

    auto c = scheduler.schedule(
        "C",
        10.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        b.depends_on(a) ==
        DependencyResult::Success
    );

    REQUIRE(
        c.depends_on(b) ==
        DependencyResult::Success
    );

    scheduler.update(0.1);

    REQUIRE(a.generation() == 1);
    REQUIRE(b.generation() == 1);
    REQUIRE(c.generation() == 1);
}


TEST_CASE("System with multiple dependencies waits for all parents") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto a = scheduler.schedule(
        "A",
        10.0,
        ExecutionSystem{},
        query
    );

    auto b = scheduler.schedule(
        "B",
        20.0,
        ExecutionSystem{},
        query
    );

    auto c = scheduler.schedule(
        "C",
        10.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        c.depends_on(a) ==
        DependencyResult::Success
    );

    REQUIRE(
        c.depends_on(b) ==
        DependencyResult::Success
    );

    // At 0.05:
    // A: not due
    // B: due
    // C: not due
    scheduler.update(0.05);

    REQUIRE(a.generation() == 0);
    REQUIRE(b.generation() == 1);
    REQUIRE(c.generation() == 0);

    // At another 0.05:
    // A: due
    // B: due
    // C: now has both generations available.
    scheduler.update(0.05);

    REQUIRE(a.generation() == 1);
    REQUIRE(b.generation() == 2);
    REQUIRE(c.generation() == 1);
}


// ============================================================
// Different frequency dependency chains
// ============================================================

TEST_CASE("Faster child waits for slower parent") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto physics = scheduler.schedule(
        "Physics",
        20.0,
        ExecutionSystem{},
        query
    );

    auto rendering = scheduler.schedule(
        "Rendering",
        60.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        rendering.depends_on(physics) ==
        DependencyResult::Success
    );

    // 1/60 second.
    scheduler.update(1.0 / 60.0);

    REQUIRE(physics.generation() == 0);
    REQUIRE(rendering.generation() == 0);

    // 1/60 more.
    scheduler.update(1.0 / 60.0);

    REQUIRE(physics.generation() == 0);
    REQUIRE(rendering.generation() == 0);

    // Third frame crosses the 20 Hz interval.
    scheduler.update(1.0 / 60.0);

    REQUIRE(physics.generation() == 1);
    REQUIRE(rendering.generation() == 1);
}


TEST_CASE("Slower child does not consume parent generation prematurely") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto physics = scheduler.schedule(
        "Physics",
        60.0,
        ExecutionSystem{},
        query
    );

    auto ai = scheduler.schedule(
        "AI",
        20.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        ai.depends_on(physics) ==
        DependencyResult::Success
    );

    scheduler.update(1.0 / 60.0);

    REQUIRE(physics.generation() == 1);
    REQUIRE(ai.generation() == 0);

    scheduler.update(1.0 / 60.0);

    REQUIRE(physics.generation() == 2);
    REQUIRE(ai.generation() == 0);

    scheduler.update(1.0 / 60.0);

    REQUIRE(physics.generation() == 3);
    REQUIRE(ai.generation() == 1);
}


// ============================================================
// Dependency graph / cycle detection
// ============================================================

TEST_CASE("Direct dependency cycle is rejected") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto a = scheduler.schedule("A", 60.0, ExecutionSystem{}, query);
    auto b = scheduler.schedule("B", 60.0, ExecutionSystem{}, query);

    REQUIRE(
        b.depends_on(a) ==
        DependencyResult::Success
    );

    REQUIRE(
        a.depends_on(b) ==
        DependencyResult::CycleDetected
    );
}


TEST_CASE("Indirect dependency cycle is rejected") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto a = scheduler.schedule("A", 60.0, ExecutionSystem{}, query);
    auto b = scheduler.schedule("B", 60.0, ExecutionSystem{}, query);
    auto c = scheduler.schedule("C", 60.0, ExecutionSystem{}, query);

    REQUIRE(
        b.depends_on(a) ==
        DependencyResult::Success
    );

    REQUIRE(
        c.depends_on(b) ==
        DependencyResult::Success
    );

    REQUIRE(
        a.depends_on(c) ==
        DependencyResult::CycleDetected
    );
}


TEST_CASE("Long dependency cycle is rejected") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto a = scheduler.schedule("A", 60.0, ExecutionSystem{}, query);
    auto b = scheduler.schedule("B", 60.0, ExecutionSystem{}, query);
    auto c = scheduler.schedule("C", 60.0, ExecutionSystem{}, query);
    auto d = scheduler.schedule("D", 60.0, ExecutionSystem{}, query);

    REQUIRE(
        b.depends_on(a) ==
        DependencyResult::Success
    );

    REQUIRE(
        c.depends_on(b) ==
        DependencyResult::Success
    );

    REQUIRE(
        d.depends_on(c) ==
        DependencyResult::Success
    );

    REQUIRE(
        a.depends_on(d) ==
        DependencyResult::CycleDetected
    );
}


// ============================================================
// Dependency graph variants
// ============================================================

TEST_CASE("Multiple children can depend on the same parent") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto parent = scheduler.schedule(
        "Physics",
        60.0,
        ExecutionSystem{},
        query
    );

    auto ai = scheduler.schedule(
        "AI",
        60.0,
        ExecutionSystem{},
        query
    );

    auto rendering = scheduler.schedule(
        "Rendering",
        60.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        ai.depends_on(parent) ==
        DependencyResult::Success
    );

    REQUIRE(
        rendering.depends_on(parent) ==
        DependencyResult::Success
    );

    scheduler.update(1.0 / 60.0);

    REQUIRE(parent.generation() == 1);
    REQUIRE(ai.generation() == 1);
    REQUIRE(rendering.generation() == 1);
}


// ============================================================
// Cancellation
// ============================================================

TEST_CASE("Cancelling a system invalidates its handle") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        60.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(system.valid());

    system.cancel();

    REQUIRE_FALSE(system.valid());
}


TEST_CASE("Cancelling a system twice is safe") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        60.0,
        ExecutionSystem{},
        query
    );

    system.cancel();
    system.cancel();

    REQUIRE_FALSE(system.valid());
}


TEST_CASE("Cancelling a parent removes it from child dependencies") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto parent = scheduler.schedule(
        "Parent",
        60.0,
        ExecutionSystem{},
        query
    );

    auto child = scheduler.schedule(
        "Child",
        60.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        child.depends_on(parent) ==
        DependencyResult::Success
    );

    parent.cancel();

    REQUIRE_FALSE(parent.valid());
    REQUIRE(child.valid());

    scheduler.update(1.0 / 60.0);

    REQUIRE(child.generation() == 1);
}


TEST_CASE("Cancelling a child preserves its parent") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto parent = scheduler.schedule(
        "Parent",
        60.0,
        ExecutionSystem{},
        query
    );

    auto child = scheduler.schedule(
        "Child",
        60.0,
        ExecutionSystem{},
        query
    );

    REQUIRE(
        child.depends_on(parent) ==
        DependencyResult::Success
    );

    child.cancel();

    REQUIRE(parent.valid());
    REQUIRE_FALSE(child.valid());

    scheduler.update(1.0 / 60.0);

    REQUIRE(parent.generation() == 1);
}


// ============================================================
// Handle movement
// ============================================================

TEST_CASE("Moving a system handle transfers ownership") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto original = scheduler.schedule(
        "System",
        60.0,
        ExecutionSystem{},
        query
    );

    const auto id = original.id();

    auto moved = std::move(original);

    REQUIRE_FALSE(original.valid());
    REQUIRE(moved.valid());
    REQUIRE(moved.id() == id);
}


TEST_CASE("Move assignment transfers system handle") {
    SimulationClock clock;
    CommandQueue queue;
    Scheduler scheduler(queue, clock);

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto first = scheduler.schedule(
        "First",
        60.0,
        ExecutionSystem{},
        query
    );

    auto second = scheduler.schedule(
        "Second",
        60.0,
        ExecutionSystem{},
        query
    );

    const auto second_id = second.id();

    first = std::move(second);

    REQUIRE(first.valid());
    REQUIRE(first.id() == second_id);
    REQUIRE_FALSE(second.valid());
}