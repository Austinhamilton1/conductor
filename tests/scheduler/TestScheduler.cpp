#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <string>

#include "scheduler/Scheduler.h"

using conductor::DependencyResult;
using conductor::Scheduler;

TEST_CASE("Scheduler executes a task at its configured frequency") {
    Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Test",
        10.0,
        [&](double) {
            ++calls;
        }
    );

    scheduler.update(0.05);

    REQUIRE(calls == 0);

    scheduler.update(0.05);

    REQUIRE(calls == 1);

    scheduler.update(0.1);

    REQUIRE(calls == 2);
}

TEST_CASE("Scheduler can run multiple independent tasks") {
    Scheduler scheduler;

    int a_calls = 0;
    int b_calls = 0;

    auto a = scheduler.schedule(
        "A",
        10.0,
        [&](double) {
            ++a_calls;
        }
    );

    auto b = scheduler.schedule(
        "B",
        20.0,
        [&](double) {
            ++b_calls;
        }
    );

    scheduler.update(0.05);

    REQUIRE(a_calls == 0);
    REQUIRE(b_calls == 1);

    scheduler.update(0.05);

    REQUIRE(a_calls == 1);
    REQUIRE(b_calls == 2);
}

TEST_CASE("TaskHandle is valid after scheduling") {
    Scheduler scheduler;

    auto handle = scheduler.schedule(
        "Test",
        10.0,
        [](double) {}
    );

    REQUIRE(handle.valid());
    REQUIRE(handle.id() != 0);
}

TEST_CASE("TaskHandle cancellation removes task") {
    Scheduler scheduler;

    int calls = 0;

    auto handle = scheduler.schedule(
        "Test",
        10.0,
        [&](double) {
            ++calls;
        }
    );

    handle.cancel();

    REQUIRE_FALSE(handle.valid());

    scheduler.update(1.0);

    REQUIRE(calls == 0);
}

TEST_CASE("Scheduler can pause a task") {
    Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Test",
        10.0,
        [&](double) {
            ++calls;
        }
    );

    task.pause();

    scheduler.update(1.0);

    REQUIRE(calls == 0);
}

TEST_CASE("Scheduler can resume a task") {
    Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Test",
        10.0,
        [&](double) {
            ++calls;
        }
    );

    task.pause();

    scheduler.update(1.0);

    REQUIRE(calls == 0);

    task.resume();

    scheduler.update(0.1);

    REQUIRE(calls == 1);
}

TEST_CASE("Scheduler can create a dependency") {
    Scheduler scheduler;

    auto parent = scheduler.schedule(
        "Parent",
        10.0,
        [](double) {}
    );

    auto child = scheduler.schedule(
        "Child",
        10.0,
        [](double) {}
    );

    REQUIRE(
        child.depends_on(parent) == DependencyResult::Success
    );
}

TEST_CASE("Scheduler rejects self dependencies") {
    Scheduler scheduler;

    auto task = scheduler.schedule(
        "Task",
        10.0,
        [](double) {}
    );

    REQUIRE(
        task.depends_on(task) == DependencyResult::SelfDependency
    );
}

TEST_CASE("Scheduler rejects invalid child") {
    Scheduler scheduler;

    auto parent = scheduler.schedule(
        "Parent",
        10.0,
        [](double) {}
    );

    REQUIRE(
        scheduler.depends_on(
            9999,
            parent.id()
        ) == DependencyResult::InvalidChild
    );
}

TEST_CASE("Scheduler rejects invalid parent") {
    Scheduler scheduler;

    auto child = scheduler.schedule(
        "Child",
        10.0,
        [](double) {}
    );

    REQUIRE(
        scheduler.depends_on(
            child.id(),
            9999
        ) == DependencyResult::InvalidParent
    );
}

TEST_CASE("Scheduler rejects duplicate dependencies") {
    Scheduler scheduler;

    auto parent = scheduler.schedule(
        "Parent",
        10.0,
        [](double) {}
    );

    auto child = scheduler.schedule(
        "Child",
        10.0,
        [](double) {}
    );

    REQUIRE(
        child.depends_on(parent) == DependencyResult::Success
    );

    REQUIRE(
        child.depends_on(parent) == DependencyResult::DuplicateDependency
    );
}

TEST_CASE("Child cannot execute before parent produces a generation") {
    Scheduler scheduler;

    int parent_calls = 0;
    int child_calls = 0;

    auto parent = scheduler.schedule(
        "Parent",
        10.0,
        [&](double) {
            ++parent_calls;
        }
    );

    auto child = scheduler.schedule(
        "Child",
        10.0,
        [&](double) {
            ++child_calls;
        }
    );

    REQUIRE(
        child.depends_on(parent) == DependencyResult::Success
    );

    scheduler.update(0.1);

    REQUIRE(parent_calls == 1);
    REQUIRE(child_calls == 1);
}

TEST_CASE("Child waits when its dependency has not generated") {
    Scheduler scheduler;

    int parent_calls = 0;
    int child_calls = 0;

    auto parent = scheduler.schedule(
        "Parent",
        1.0,
        [&](double) {
            ++parent_calls;
        }
    );

    auto child = scheduler.schedule(
        "Child",
        10.0,
        [&](double) {
            ++child_calls;
        }
    );

    child.depends_on(parent);

    scheduler.update(0.1);

    REQUIRE(parent_calls == 0);
    REQUIRE(child_calls == 0);

    scheduler.update(0.9);

    REQUIRE(parent_calls == 1);
    REQUIRE(child_calls == 1);
}

TEST_CASE("One parent generation permits one child execution") {
    Scheduler scheduler;

    int parent_calls = 0;
    int child_calls = 0;

    auto parent = scheduler.schedule(
        "Parent",
        1.0,
        [&](double) {
            ++parent_calls;
        }
    );

    auto child = scheduler.schedule(
        "Child",
        100.0,
        [&](double) {
            ++child_calls;
        }
    );

    child.depends_on(parent);

    // Parent produces generation 1.
    scheduler.update(1.0);

    REQUIRE(parent_calls == 1);
    REQUIRE(child_calls == 1);

    // Child is extremely fast, but parent has not
    // produced another generation.
    scheduler.update(1.0);

    REQUIRE(parent_calls == 2);
    REQUIRE(child_calls == 2);
}

TEST_CASE("Dependency limits a faster child") {
    Scheduler scheduler;

    int parent_calls = 0;
    int child_calls = 0;

    auto parent = scheduler.schedule(
        "Parent",
        10.0,
        [&](double) {
            ++parent_calls;
        }
    );

    auto child = scheduler.schedule(
        "Child",
        100.0,
        [&](double) {
            ++child_calls;
        }
    );

    child.depends_on(parent);

    scheduler.update(0.1);

    REQUIRE(parent_calls == 1);
    REQUIRE(child_calls == 1);

    scheduler.update(0.1);

    REQUIRE(parent_calls == 2);
    REQUIRE(child_calls == 2);

    scheduler.update(0.1);

    REQUIRE(parent_calls == 3);
    REQUIRE(child_calls == 3);
}

TEST_CASE("Dependency does not force a slower child to run faster") {
    Scheduler scheduler;

    int parent_calls = 0;
    int child_calls = 0;

    auto parent = scheduler.schedule(
        "Parent",
        100.0,
        [&](double) {
            ++parent_calls;
        }
    );

    auto child = scheduler.schedule(
        "Child",
        10.0,
        [&](double) {
            ++child_calls;
        }
    );

    child.depends_on(parent);

    scheduler.update(0.1);

    REQUIRE(parent_calls == 10);
    REQUIRE(child_calls == 1);

    scheduler.update(0.1);

    REQUIRE(parent_calls == 20);
    REQUIRE(child_calls == 2);

    scheduler.update(0.1);

    REQUIRE(parent_calls == 30);
    REQUIRE(child_calls == 3);
}

TEST_CASE("Child requires a fresh generation from every dependency") {
    Scheduler scheduler;

    int a_calls = 0;
    int b_calls = 0;
    int c_calls = 0;

    auto a = scheduler.schedule(
        "A",
        10.0,
        [&](double) {
            ++a_calls;
        }
    );

    auto b = scheduler.schedule(
        "B",
        10.0,
        [&](double) {
            ++b_calls;
        }
    );

    auto c = scheduler.schedule(
        "C",
        10.0,
        [&](double) {
            ++c_calls;
        }
    );

    scheduler.depends_on(c.id(), a.id());
    scheduler.depends_on(c.id(), b.id());

    scheduler.update(0.1);

    REQUIRE(a_calls == 1);
    REQUIRE(b_calls == 1);

    // Both dependencies have generated.
    REQUIRE(c_calls == 1);
}

TEST_CASE("Child remains blocked if one dependency has not generated") {
    Scheduler scheduler;

    int a_calls = 0;
    int b_calls = 0;
    int c_calls = 0;

    auto a = scheduler.schedule(
        "A",
        10.0,
        [&](double) {
            ++a_calls;
        }
    );

    auto b = scheduler.schedule(
        "B",
        1.0,
        [&](double) {
            ++b_calls;
        }
    );

    auto c = scheduler.schedule(
        "C",
        10.0,
        [&](double) {
            ++c_calls;
        }
    );

    scheduler.depends_on(c.id(), a.id());
    scheduler.depends_on(c.id(), b.id());

    scheduler.update(0.1);

    REQUIRE(a_calls == 1);
    REQUIRE(b_calls == 0);
    REQUIRE(c_calls == 0);

    scheduler.update(0.9);

    REQUIRE(a_calls == 10);
    REQUIRE(b_calls == 1);
    REQUIRE(c_calls == 1);
}

TEST_CASE("Dependency chain executes in order") {
    Scheduler scheduler;

    std::vector<std::string> execution_order;

    auto a = scheduler.schedule(
        "A",
        10.0,
        [&](double) {
            execution_order.push_back("A");
        }
    );

    auto b = scheduler.schedule(
        "B",
        10.0,
        [&](double) {
            execution_order.push_back("B");
        }
    );

    auto c = scheduler.schedule(
        "C",
        10.0,
        [&](double) {
            execution_order.push_back("C");
        }
    );

    scheduler.depends_on(b.id(), a.id());
    scheduler.depends_on(c.id(), b.id());

    scheduler.update(0.1);

    REQUIRE(execution_order.size() == 3);

    REQUIRE(execution_order[0] == "A");
    REQUIRE(execution_order[1] == "B");
    REQUIRE(execution_order[2] == "C");
}

TEST_CASE("Scheduler rejects dependency cycles") {
    Scheduler scheduler;

    auto a = scheduler.schedule(
        "A",
        10.0,
        [](double) {}
    );

    auto b = scheduler.schedule(
        "B",
        10.0,
        [](double) {}
    );

    auto c = scheduler.schedule(
        "C",
        10.0,
        [](double) {}
    );

    REQUIRE(
        b.depends_on(a)
        == DependencyResult::Success
    );

    REQUIRE(
        c.depends_on(b)
        == DependencyResult::Success
    );

    REQUIRE(
        a.depends_on(c)
        == DependencyResult::CycleDetected
    );
}

TEST_CASE("Scheduler rejects direct dependency cycle") {
    Scheduler scheduler;

    auto a = scheduler.schedule(
        "A",
        10.0,
        [](double) {}
    );

    auto b = scheduler.schedule(
        "B",
        10.0,
        [](double) {}
    );

    REQUIRE(
        scheduler.depends_on(b.id(), a.id())
        == DependencyResult::Success
    );

    REQUIRE(
        scheduler.depends_on(a.id(), b.id())
        == DependencyResult::CycleDetected
    );
}

TEST_CASE("Independent tasks do not block each other") {
    Scheduler scheduler;

    int a_calls = 0;
    int b_calls = 0;

    auto a = scheduler.schedule(
        "A",
        1.0,
        [&](double) {
            ++a_calls;
        }
    );

    auto b = scheduler.schedule(
        "B",
        10.0,
        [&](double) {
            ++b_calls;
        }
    );

    scheduler.update(0.1);

    REQUIRE(a_calls == 0);
    REQUIRE(b_calls == 1);

    scheduler.update(0.9);

    REQUIRE(a_calls == 1);
    REQUIRE(b_calls == 10);
}

TEST_CASE("Cancelling a parent removes its dependency from child") {
    Scheduler scheduler;

    int child_calls = 0;

    auto parent = scheduler.schedule(
        "Parent",
        10.0,
        [](double) {}
    );

    auto child = scheduler.schedule(
        "Child",
        10.0,
        [&](double) {
            ++child_calls;
        }
    );

    scheduler.depends_on(
        child.id(),
        parent.id()
    );

    parent.cancel();

    scheduler.update(1.0);

    // The dependency should have been removed.
    REQUIRE(child_calls > 0);
}

TEST_CASE("Paused parent prevents child from receiving new generations") {
    Scheduler scheduler;

    int parent_calls = 0;
    int child_calls = 0;

    auto parent = scheduler.schedule(
        "Parent",
        10.0,
        [&](double) {
            ++parent_calls;
        }
    );

    auto child = scheduler.schedule(
        "Child",
        10.0,
        [&](double) {
            ++child_calls;
        }
    );

    scheduler.depends_on(
        child.id(),
        parent.id()
    );

    scheduler.update(0.1);

    REQUIRE(parent_calls == 1);
    REQUIRE(child_calls == 1);

    parent.pause();

    scheduler.update(1.0);

    REQUIRE(parent_calls == 1);
    REQUIRE(child_calls == 1);

    parent.resume();

    scheduler.update(0.1);

    REQUIRE(parent_calls == 2);
    REQUIRE(child_calls == 2);
}

TEST_CASE("Child consumes parent generation after execution") {
    Scheduler scheduler;

    int child_calls = 0;

    auto parent = scheduler.schedule(
        "Parent",
        1.0,
        [](double) {}
    );

    auto child = scheduler.schedule(
        "Child",
        100.0,
        [&](double) {
            ++child_calls;
        }
    );

    scheduler.depends_on(
        child.id(),
        parent.id()
    );

    scheduler.update(1.0);

    REQUIRE(child_calls == 1);

    // Parent hasn't generated anything new.
    scheduler.update(0.01);
    scheduler.update(0.01);
    scheduler.update(0.01);

    REQUIRE(child_calls == 1);
}

TEST_CASE("Blocked child does not execute stale accumulated work") {
    Scheduler scheduler;

    int parent_calls = 0;
    int child_calls = 0;

    auto parent = scheduler.schedule(
        "Parent",
        1.0,
        [&](double) {
            ++parent_calls;
        }
    );

    auto child = scheduler.schedule(
        "Child",
        100.0,
        [&](double) {
            ++child_calls;
        }
    );

    scheduler.depends_on(
        child.id(),
        parent.id()
    );

    // Child becomes very far behind while parent
    // has produced nothing.
    scheduler.update(0.5);

    REQUIRE(parent_calls == 0);
    REQUIRE(child_calls == 0);

    // Parent generates exactly once.
    scheduler.update(0.5);

    REQUIRE(parent_calls == 1);
    REQUIRE(child_calls == 1);

    // Even though the child has accumulated a lot
    // of elapsed time, it cannot execute again without
    // another parent generation.
    scheduler.update(0.01);

    REQUIRE(child_calls == 1);
}

TEST_CASE("Changing task frequency affects scheduling") {
    Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Test",
        10.0,
        [&](double) {
            ++calls;
        }
    );

    scheduler.update(0.1);

    REQUIRE(calls == 1);

    task.set_frequency(20.0);

    scheduler.update(0.05);

    REQUIRE(calls == 2);
}

TEST_CASE("Failed task does not produce a generation") {
    Scheduler scheduler;

    int child_calls = 0;

    auto parent = scheduler.schedule(
        "Parent",
        10.0,
        [](double) {
            throw std::runtime_error("failure");
        }
    );

    auto child = scheduler.schedule(
        "Child",
        10.0,
        [&](double) {
            ++child_calls;
        }
    );

    scheduler.depends_on(
        child.id(),
        parent.id()
    );

    REQUIRE_THROWS(
        scheduler.update(0.1)
    );

    REQUIRE(child_calls == 0);
}

TEST_CASE("Scheduler handles branching and converging dependency graph") {
    Scheduler scheduler;

    std::vector<std::string> execution;

    auto sensor = scheduler.schedule(
        "Sensor",
        10.0,
        [&](double) {
            execution.push_back("Sensor");
        }
    );

    auto physics = scheduler.schedule(
        "Physics",
        10.0,
        [&](double) {
            execution.push_back("Physics");
        }
    );

    auto controller = scheduler.schedule(
        "Controller",
        10.0,
        [&](double) {
            execution.push_back("Controller");
        }
    );

    auto renderer = scheduler.schedule(
        "Renderer",
        10.0,
        [&](double) {
            execution.push_back("Renderer");
        }
    );

    scheduler.depends_on(physics.id(), sensor.id());
    scheduler.depends_on(controller.id(), sensor.id());
    scheduler.depends_on(renderer.id(), physics.id());
    scheduler.depends_on(renderer.id(), controller.id());

    scheduler.update(0.1);

    REQUIRE(execution.size() == 4);

    // Sensor must happen first.
    REQUIRE(execution[0] == "Sensor");

    // Physics and Controller can occur in either order.
    REQUIRE(
        (execution[1] == "Physics" ||
         execution[1] == "Controller")
    );

    REQUIRE(
        (execution[2] == "Physics" ||
         execution[2] == "Controller")
    );

    REQUIRE(execution[3] == "Renderer");
}