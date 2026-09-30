#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <stdexcept>
#include <utility>

#include <cmath>

#include "scheduler/Scheduler.h"
#include "commands/CommandQueue.h"
#include "core/SimulationClock.h"
#include "core/Random.h"

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

    struct Health {
        float hp;
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
                ctx.delta_seconds();
        }

        static void reset() {
            executions = 0;
            last_delta = 0.0;
        }
    };

    struct ReaderSystem {
        using Query = conductor::Query<
            conductor::Read<Position>
        >;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>& ctx
        ) {
            return;
        }
    };

    struct WriterSystem {
        using Query = conductor::Query<
            conductor::Write<Position>
        >;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>& ctx
        ) {
            return;
        }
    };

    struct PhysicsSystem {
        using Query = conductor::Query<
            conductor::Write<Position>,
            conductor::Read<Velocity>
        >;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>& ctx
        ) {
            return;
        }
    };

    struct RenderSystem {
        using Query = conductor::Query<
            conductor::Read<Position>,
            conductor::Read<Velocity>
        >;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>& ctx
        ) {
            return;
        }
    };

    struct MovementSystem {
        using Query = conductor::Query<
            conductor::Read<Position>,
            conductor::Write<Velocity>
        >;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>& ctx
        ) {
            return;
        }
    };

    struct ConsumerSystem {
        using Query = conductor::Query<
            conductor::Read<Velocity>
        >;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>& ctx
        ) {
            return;
        }
    };

    struct HealthSystem {
        using Query = conductor::Query<
            conductor::Read<Position>,
            conductor::Write<Health>
        >;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>& ctx
        ) {
            return;
        }
    };

    struct VigorSystem {
        using Query = conductor::Query<
            conductor::Read<Health>,
            conductor::Write<Velocity>
        >;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>& ctx
        ) {
            return;
        }
    };
}

using namespace conductor;

// ============================================================
// Scheduling
// ============================================================

TEST_CASE("Scheduler creates a valid system handle", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };
    
    Scheduler scheduler;

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


TEST_CASE("Scheduler assigns unique system IDs", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };

    Scheduler scheduler;

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


TEST_CASE("Scheduler rejects zero frequency", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };
    
    Scheduler scheduler;

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


TEST_CASE("Scheduler rejects negative frequency", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

TEST_CASE("System does not execute before its interval", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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
    scheduler.update(0.05, ctx);

    REQUIRE(system.generation() == 0);
    REQUIRE(system.state() == SystemState::Waiting);

    scheduler.update(0.049, ctx);

    REQUIRE(system.generation() == 0);
}


TEST_CASE("System executes when its interval is reached", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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
    scheduler.update(0.1, ctx);

    REQUIRE(system.generation() == 1);
    REQUIRE(system.state() == SystemState::Waiting);
}


TEST_CASE("System accumulates delta across updates", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        10.0,
        ExecutionSystem{},
        query
    );

    scheduler.update(0.03, ctx);

    REQUIRE(system.generation() == 0);

    scheduler.update(0.03, ctx);

    REQUIRE(system.generation() == 0);

    scheduler.update(0.04, ctx);

    REQUIRE(system.generation() == 1);
}


TEST_CASE("System can execute multiple times from accumulated delta", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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
    scheduler.update(0.35, ctx);

    REQUIRE(system.generation() == 3);
}


TEST_CASE("System retains fractional accumulated time", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        10.0,
        ExecutionSystem{},
        query
    );

    scheduler.update(0.35, ctx);

    REQUIRE(system.generation() == 3);

    scheduler.update(0.05, ctx);

    REQUIRE(system.generation() == 4);
}


TEST_CASE("System frequency can be changed", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

    scheduler.update(0.05, ctx);

    REQUIRE(system.generation() == 1);
}


TEST_CASE("Invalid frequency change throws", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

TEST_CASE("Paused system does not accumulate time", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

    scheduler.update(1.0, ctx);

    REQUIRE(system.generation() == 0);
}


TEST_CASE("Resuming system allows scheduling to continue", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

    scheduler.update(1.0, ctx);

    REQUIRE(system.generation() == 0);

    system.resume();

    REQUIRE_FALSE(system.paused());

    scheduler.update(0.1, ctx);

    REQUIRE(system.generation() == 1);
}


TEST_CASE("Pausing preserves accumulated time", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(positions);

    auto system = scheduler.schedule(
        "System",
        10.0,
        ExecutionSystem{},
        query
    );

    scheduler.update(0.05, ctx);

    system.pause();

    scheduler.update(1.0, ctx);

    REQUIRE(system.generation() == 0);

    system.resume();

    scheduler.update(0.05, ctx);

    REQUIRE(system.generation() == 1);
}


// ============================================================
// Dependencies
// ============================================================

TEST_CASE("System can depend on another system", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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


TEST_CASE("System cannot depend on itself", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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


TEST_CASE("Duplicate dependency is rejected", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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


TEST_CASE("Dependency between different schedulers is rejected", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };    
    CommandQueue first_queue;
    Scheduler first;

    Scheduler second;

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

TEST_CASE("Child waits for parent generation", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

    scheduler.update(0.1, ctx);

    REQUIRE(parent.generation() == 1);
    REQUIRE(child.generation() == 1);
}


TEST_CASE("Child cannot execute twice without a new parent generation") { 
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

    scheduler.update(0.1, ctx);

    REQUIRE(parent.generation() == 1);
    REQUIRE(child.generation() == 1);

    // Both are now waiting for another interval.
    scheduler.update(0.1, ctx);

    REQUIRE(parent.generation() == 2);
    REQUIRE(child.generation() == 2);
}


TEST_CASE("Dependency chain executes in generation order", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

    scheduler.update(0.1, ctx);

    REQUIRE(a.generation() == 1);
    REQUIRE(b.generation() == 1);
    REQUIRE(c.generation() == 1);
}


TEST_CASE("System with multiple dependencies waits for all parents", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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
    scheduler.update(0.05, ctx);

    REQUIRE(a.generation() == 0);
    REQUIRE(b.generation() == 1);
    REQUIRE(c.generation() == 0);

    // At another 0.05:
    // A: due
    // B: due
    // C: now has both generations available.
    scheduler.update(0.05, ctx);

    REQUIRE(a.generation() == 1);
    REQUIRE(b.generation() == 2);
    REQUIRE(c.generation() == 1);
}


// ============================================================
// Different frequency dependency chains
// ============================================================

TEST_CASE("Faster child waits for slower parent", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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
    scheduler.update(1.0 / 60.0, ctx);

    REQUIRE(physics.generation() == 0);
    REQUIRE(rendering.generation() == 0);

    // 1/60 more.
    scheduler.update(1.0 / 60.0, ctx);

    REQUIRE(physics.generation() == 0);
    REQUIRE(rendering.generation() == 0);

    // Third frame crosses the 20 Hz interval.
    scheduler.update(1.0 / 60.0, ctx);

    REQUIRE(physics.generation() == 1);
    REQUIRE(rendering.generation() == 1);
}


TEST_CASE("Slower child does not consume parent generation prematurely", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

    scheduler.update(1.0 / 60.0, ctx);

    REQUIRE(physics.generation() == 1);
    REQUIRE(ai.generation() == 0);

    scheduler.update(1.0 / 60.0, ctx);

    REQUIRE(physics.generation() == 2);
    REQUIRE(ai.generation() == 0);

    scheduler.update(1.0 / 60.0, ctx);

    REQUIRE(physics.generation() == 3);
    REQUIRE(ai.generation() == 1);
}


// ============================================================
// Dependency graph / cycle detection
// ============================================================

TEST_CASE("Direct dependency cycle is rejected", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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


TEST_CASE("Indirect dependency cycle is rejected", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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


TEST_CASE("Long dependency cycle is rejected", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

TEST_CASE("Multiple children can depend on the same parent", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

    scheduler.update(1.0 / 60.0, ctx);

    REQUIRE(parent.generation() == 1);
    REQUIRE(ai.generation() == 1);
    REQUIRE(rendering.generation() == 1);
}


// ============================================================
// Cancellation
// ============================================================

TEST_CASE("Cancelling a system invalidates its handle", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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


TEST_CASE("Cancelling a system twice is safe", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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


TEST_CASE("Cancelling a parent removes it from child dependencies", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

    scheduler.update(1.0 / 60.0, ctx);

    REQUIRE(child.generation() == 1);
}


TEST_CASE("Cancelling a child preserves its parent", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

    scheduler.update(1.0 / 60.0, ctx);

    REQUIRE(parent.generation() == 1);
}


// ============================================================
// Handle movement
// ============================================================

TEST_CASE("Moving a system handle transfers ownership", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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


TEST_CASE("Move assignment transfers system handle", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

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

TEST_CASE("Read/Read access does not create dependency", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> storage;

    auto a = scheduler.schedule(
        "reader_a",
        60.0,
        ReaderSystem{},
        Query<Read<Position>>{}.bind_storage(storage)
    );

    auto b = scheduler.schedule(
        "reader_b",
        60.00,
        ReaderSystem{},
        Query<Read<Position>>{}.bind_storage(storage)
    );

    scheduler.resolve_dependencies();

    REQUIRE_FALSE(scheduler.has_dependency_path(b.id(), a.id()));
    REQUIRE_FALSE(scheduler.has_dependency_path(a.id(), b.id()));
}

TEST_CASE("Write/Read access infers writer before reader", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> storage;

    auto writer = scheduler.schedule(
        "writer",
        60.0,
        WriterSystem{},
        Query<Write<Position>>{}.bind_storage(storage)
    );

    auto reader = scheduler.schedule(
        "reader",
        60.0,
        ReaderSystem{},
        Query<Read<Position>>{}.bind_storage(storage)
    );

    scheduler.resolve_dependencies();

    REQUIRE(scheduler.has_dependency_path(
        reader.id(),
        writer.id()
    ));

    REQUIRE_FALSE(scheduler.has_dependency_path(
        writer.id(),
        reader.id()
    ));
}

TEST_CASE("Read/Write access infers writer before reader", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> storage;

    auto reader = scheduler.schedule(
        "reader",
        60.0,
        ReaderSystem{},
        Query<Read<Position>>{}.bind_storage(storage)
    );

    auto writer = scheduler.schedule(
        "writer",
        60.0,
        WriterSystem{},
        Query<Write<Position>>{}.bind_storage(storage)
    );

    scheduler.resolve_dependencies();

    REQUIRE(scheduler.has_dependency_path(
        reader.id(),
        writer.id()
    ));

    REQUIRE_FALSE(scheduler.has_dependency_path(
        writer.id(),
        reader.id()
    ));
}

TEST_CASE("Write/Write access requires explicit dependency", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> storage;

    auto a = scheduler.schedule(
        "writer_a",
        60.0,
        WriterSystem{},
        Query<Write<Position>>{}.bind_storage(storage)
    );

    auto b = scheduler.schedule(
        "writer_b",
        60.0,
        WriterSystem{},
        Query<Write<Position>>{}.bind_storage(storage)
    );

    REQUIRE_THROWS(
        scheduler.resolve_dependencies()
    );
}

TEST_CASE("Explicit dependency resolves write/write conflict", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> storage;

    auto a = scheduler.schedule(
        "writer_a",
        60.0,
        WriterSystem{},
        Query<Write<Position>>{}.bind_storage(storage)
    );

    auto b = scheduler.schedule(
        "writer_b",
        60.0,
        WriterSystem{},
        Query<Write<Position>>{}.bind_storage(storage)
    );

    REQUIRE(
        b.depends_on(a) == DependencyResult::Success
    );

    REQUIRE_NOTHROW(
        scheduler.resolve_dependencies()
    );

    REQUIRE(scheduler.has_dependency_path(
        b.id(),
        a.id()
    ));
}

TEST_CASE("Explicit dependency can resolve write/write in either direction", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> storage;

    auto a = scheduler.schedule(
        "writer_a",
        60.0,
        WriterSystem{},
        Query<Write<Position>>{}.bind_storage(storage)
    );

    auto b = scheduler.schedule(
        "writer_b",
        60.0,
        WriterSystem{},
        Query<Write<Position>>{}.bind_storage(storage)
    );

    REQUIRE(
        a.depends_on(b) == DependencyResult::Success
    );

    REQUIRE_NOTHROW(
        scheduler.resolve_dependencies()
    );
}

TEST_CASE("Accesses to different components do not create dependency", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> positions;
    ComponentStorage<Velocity> velocities;

    auto position_writer = scheduler.schedule(
        "position_writer",
        60.0,
        WriterSystem{},
        Query<Write<Position>>{}.bind_storage(positions)
    );

    auto velocity_writer = scheduler.schedule(
        "velocity_writer",
        60.0,
        WriterSystem{},
        Query<Write<Velocity>>{}.bind_storage(velocities)
    );

    scheduler.resolve_dependencies();

    REQUIRE_FALSE(scheduler.has_dependency_path(
        position_writer.id(),
        velocity_writer.id()
    ));

    REQUIRE_FALSE(scheduler.has_dependency_path(
        velocity_writer.id(),
        position_writer.id()
    ));
}

TEST_CASE("Mixed query infers dependencies for conflicting components", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> positions;
    ComponentStorage<Velocity> velocities;

    auto physics = scheduler.schedule(
        "physics",
        60.0,
        PhysicsSystem{},
        Query<
            Read<Velocity>,
            Write<Position>
        >{}.bind_storage(velocities, positions)
    );

    auto renderer = scheduler.schedule(
        "renderer",
        60.0,
        RenderSystem{},
        Query<
            Read<Position>,
            Read<Velocity>
        >{}.bind_storage(positions, velocities)
    );

    scheduler.resolve_dependencies();

    REQUIRE(scheduler.has_dependency_path(
        renderer.id(),
        physics.id()
    ));
}

TEST_CASE("Inferred dependencies form transitive ordering", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> positions;
    ComponentStorage<Velocity> velocities;

    auto physics = scheduler.schedule(
        "physics",
        60.0,
        PhysicsSystem{},
        Query<Write<Position>>{}.bind_storage(positions)
    );

    auto movement = scheduler.schedule(
        "movement",
        60.0,
        MovementSystem{},
        Query<
            Read<Position>,
            Write<Velocity>
        >{}.bind_storage(positions, velocities)
    );

    auto consumer = scheduler.schedule(
        "consumer",
        60.0,
        ConsumerSystem{},
        Query<Read<Velocity>>{}.bind_storage(velocities)
    );

    scheduler.resolve_dependencies();

    REQUIRE(scheduler.has_dependency_path(
        movement.id(),
        physics.id()
    ));

    REQUIRE(scheduler.has_dependency_path(
        consumer.id(),
        movement.id()
    ));

    REQUIRE(scheduler.has_dependency_path(
        consumer.id(),
        physics.id()
    ));
}

TEST_CASE("Conflicting access pattern that creates cycle is rejected", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> positions;
    ComponentStorage<Velocity> velocities;
    ComponentStorage<Health> health;

    auto a = scheduler.schedule(
        "system_a",
        60.0,
        PhysicsSystem{},
        Query<
            Write<Position>,
            Read<Velocity>
        >{}.bind_storage(positions, velocities)
    );

    auto b = scheduler.schedule(
        "system_b",
        60.0,
        HealthSystem{},
        Query<
            Read<Position>,
            Write<Health>
        >{}.bind_storage(positions, health)
    );

    auto c = scheduler.schedule(
        "system_c",
        60.0,
        PhysicsSystem{},
        Query<
            Read<Health>,
            Write<Velocity>
        >{}.bind_storage(health, velocities)
    );

    REQUIRE_THROWS(
        scheduler.resolve_dependencies()
    );
}

TEST_CASE("2 cycle dependency throws", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> positions;
    ComponentStorage<Velocity> velocities;

    auto physics = scheduler.schedule(
        "physics",
        60.0,
        PhysicsSystem{},
        Query<
            Write<Position>,
            Read<Velocity>
        >{}.bind_storage(positions, velocities)
    );

    auto movement = scheduler.schedule(
        "movement",
        60.0,
        MovementSystem{},
        Query<
            Read<Position>,
            Write<Velocity>
        >{}.bind_storage(positions, velocities)
    );

    REQUIRE_THROWS(
        scheduler.resolve_dependencies()
    );
}

TEST_CASE("2 cycle dependency can be resolved with explicit dependency", "[scheduler]") {
    SimulationClock clock;
    Random rand;
    CommandQueue commands;
    EventBus events;
    
    WorldContext ctx{
        .rand = rand,
        .clock = clock,
        .queue = commands,
        .events = events,
    };	
    
    Scheduler scheduler;

    ComponentStorage<Position> positions;
    ComponentStorage<Velocity> velocities;

    auto physics = scheduler.schedule(
        "physics",
        60.0,
        PhysicsSystem{},
        Query<
            Write<Position>,
            Read<Velocity>
        >{}.bind_storage(positions, velocities)
    );

    auto movement = scheduler.schedule(
        "movement",
        60.0,
        MovementSystem{},
        Query<
            Read<Position>,
            Write<Velocity>
        >{}.bind_storage(positions, velocities)
    );

    physics.depends_on(movement);

    REQUIRE_NOTHROW(
        scheduler.resolve_dependencies()
    );
}