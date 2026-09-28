#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "scheduler/Scheduler.h"

#include "commands/CommandQueue.h"
#include "core/SimulationClock.h"

#include "ecs/ComponentStorage.h"
#include "ecs/Query.h"

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

    struct OrderSystemA {
        using Query =
            conductor::Query<conductor::Read<Position>>;

        inline static std::vector<int>* order =
            nullptr;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>&
        ) {
            order->push_back(1);
        }
    };


    struct OrderSystemB {
        using Query =
            conductor::Query<conductor::Read<Position>>;

        inline static std::vector<int>* order =
            nullptr;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>&
        ) {
            order->push_back(2);
        }
    };

    struct FailingSystem {
        using Query = conductor::Query<
            conductor::Read<Position>
        >;

        inline static int executions = 0;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>&
        ) {
            ++executions;

            throw std::runtime_error(
                "intentional system failure"
            );
        }

        static void reset() {
            executions = 0;
        }
    };

    struct DependentSystem {
        using Query =
            conductor::Query<conductor::Read<Position>>;

        inline static int executions = 0;

        template<typename BoundQuery>
        void update(
            conductor::SystemContext<BoundQuery>&
        ) {
            ++executions;
        }

        static void reset() {
            executions = 0;
        }
    };
}

using namespace conductor;

TEST_CASE("Scheduler executes systems when due", "[scheduler]") {
    ExecutionSystem::reset();

    CommandQueue commands;
    SimulationClock clock;
	Random rand;

    Scheduler scheduler(
        commands,
        clock
    );

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(
            positions
        );

    auto handle =
        scheduler.schedule(
            "Execution",
            10.0,
            ExecutionSystem{},
            std::move(query),
            rand
        );

    REQUIRE(handle.valid());

    scheduler.update(0.05);

    REQUIRE(
        ExecutionSystem::executions == 0
    );

    scheduler.update(0.05);

    REQUIRE(
        ExecutionSystem::executions == 1
    );
}

TEST_CASE("Scheduler accumulates partial intervals", "[scheduler]") {
    ExecutionSystem::reset();

    CommandQueue commands;
    SimulationClock clock;
	Random rand;

    Scheduler scheduler(
        commands,
        clock
    );

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(
            positions
        );

    auto handle = scheduler.schedule(
        "Execution",
        10.0,
        ExecutionSystem{},
        std::move(query),
        rand
    );

    scheduler.update(0.03);
    scheduler.update(0.03);
    scheduler.update(0.03);

    REQUIRE(
        ExecutionSystem::executions == 0
    );

    scheduler.update(0.01);

    REQUIRE(
        ExecutionSystem::executions == 1
    );
}

TEST_CASE("Scheduler executes multiple intervals from large delta", "[scheduler]") {
    ExecutionSystem::reset();

    CommandQueue commands;
    SimulationClock clock;
	Random rand;

    Scheduler scheduler(
        commands,
        clock
    );

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(
            positions
        );

    auto handle = scheduler.schedule(
        "Execution",
        10.0,
        ExecutionSystem{},
        std::move(query),
        rand
    );

    scheduler.update(0.35);

    REQUIRE(
        ExecutionSystem::executions == 3
    );
}

TEST_CASE("Scheduler pauses and resumes systems", "[scheduler]") {
    ExecutionSystem::reset();

    CommandQueue commands;
    SimulationClock clock;
	Random rand;

    Scheduler scheduler(
        commands,
        clock
    );

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(
            positions
        );

    auto handle =
        scheduler.schedule(
            "Execution",
            10.0,
            ExecutionSystem{},
            std::move(query),
            rand
        );

    handle.pause();

    scheduler.update(1.0);

    REQUIRE(
        ExecutionSystem::executions == 0
    );

    handle.resume();

    scheduler.update(0.1);

    REQUIRE(
        ExecutionSystem::executions == 1
    );
}

TEST_CASE("Cancelling a system prevents future execution", "[scheduler]") {
    ExecutionSystem::reset();

    CommandQueue commands;
    SimulationClock clock;
	Random rand;

    Scheduler scheduler(
        commands,
        clock
    );

    ComponentStorage<Position> positions;

    auto query =
        ExecutionSystem::Query::bind_storage(
            positions
        );

    auto handle =
        scheduler.schedule(
            "Execution",
            10.0,
            ExecutionSystem{},
            std::move(query),
            rand
        );

    REQUIRE(handle.valid());

    handle.cancel();

    REQUIRE_FALSE(handle.valid());

    scheduler.update(1.0);

    REQUIRE(
        ExecutionSystem::executions == 0
    );
}

TEST_CASE("Scheduler respects system dependencies", "[scheduler]") {
    std::vector<int> order;

    OrderSystemA::order = &order;
    OrderSystemB::order = &order;

    CommandQueue commands;
    SimulationClock clock;
	Random rand;

    Scheduler scheduler(
        commands,
        clock
    );

    ComponentStorage<Position> positions;

    auto query_a =
        OrderSystemA::Query::bind_storage(
            positions
        );

    auto query_b =
        OrderSystemB::Query::bind_storage(
            positions
        );

    auto a =
        scheduler.schedule(
            "A",
            60.0,
            OrderSystemA{},
            std::move(query_a),
            rand
        );

    auto b =
        scheduler.schedule(
            "B",
            60.0,
            OrderSystemB{},
            std::move(query_b),
            rand
        );

    REQUIRE(
        b.depends_on(a)
        == DependencyResult::Success
    );

    scheduler.update(
        1.0 / 60.0
    );

    REQUIRE(order.size() == 2);

    REQUIRE(order[0] == 1);
    REQUIRE(order[1] == 2);
}

TEST_CASE("System dependency requires parent execution", "[scheduler]") {
    std::vector<int> order;

    OrderSystemA::order = &order;
    OrderSystemB::order = &order;

    CommandQueue commands;
    SimulationClock clock;
	Random rand;

    Scheduler scheduler(
        commands,
        clock
    );

    ComponentStorage<Position> positions;

    auto query_a =
        OrderSystemA::Query::bind_storage(
            positions
        );

    auto query_b =
        OrderSystemB::Query::bind_storage(
            positions
        );

    auto a =
        scheduler.schedule(
            "A",
            60.0,
            OrderSystemA{},
            std::move(query_a),
            rand
        );

    auto b =
        scheduler.schedule(
            "B",
            120.0,
            OrderSystemB{},
            std::move(query_b),
            rand
        );

    REQUIRE(
        b.depends_on(a)
        == DependencyResult::Success
    );

    scheduler.update(
        1.0 / 60.0
    );

    REQUIRE(order.size() == 2);

    order.clear();

    // B must not run again merely because it is due.
    scheduler.update(
        1.0 / 120.0
    );

    REQUIRE(order.empty());
}

TEST_CASE("Failed system enters failed state", "[scheduler]") {
    FailingSystem::reset();

    CommandQueue commands;
    SimulationClock clock;
	Random rand;

    Scheduler scheduler(
        commands,
        clock
    );

    ComponentStorage<Position> positions;

    auto query =
        FailingSystem::Query::bind_storage(
            positions
        );

    auto handle =
        scheduler.schedule(
            "Failure",
            60.0,
            FailingSystem{},
            std::move(query),
            rand
        );

    REQUIRE_THROWS(
        scheduler.update(
            1.0 / 60.0
        )
    );

    REQUIRE(
        FailingSystem::executions == 1
    );

    // The system should not be retried automatically.
    REQUIRE_NOTHROW(
        scheduler.update(
            1.0 / 60.0
        )
    );

    REQUIRE(
        FailingSystem::executions == 1
    );
}

TEST_CASE("Failed system does not satisfy dependencies", "[scheduler]") {
    FailingSystem::reset();
    DependentSystem::reset();

    CommandQueue commands;
    SimulationClock clock;
	Random rand;

    Scheduler scheduler(
        commands,
        clock
    );

    ComponentStorage<Position> positions;

    auto parent_query =
        FailingSystem::Query::bind_storage(
            positions
        );

    auto child_query =
        DependentSystem::Query::bind_storage(
            positions
        );

    auto parent =
        scheduler.schedule(
            "Parent",
            60.0,
            FailingSystem{},
            std::move(parent_query),
            rand
        );

    auto child =
        scheduler.schedule(
            "Child",
            60.0,
            DependentSystem{},
            std::move(child_query),
            rand
        );

    REQUIRE(
        child.depends_on(parent)
        == DependencyResult::Success
    );

    REQUIRE_THROWS(
        scheduler.update(
            1.0 / 60.0
        )
    );

    REQUIRE(
        FailingSystem::executions == 1
    );

    REQUIRE(
        DependentSystem::executions == 0
    );
}