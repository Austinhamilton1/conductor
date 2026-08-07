#include <catch2/catch_test_macros.hpp>

#include "scheduler/Scheduler.h"

TEST_CASE("Scheduler starts with no tasks") {
    conductor::Scheduler scheduler;

    scheduler.update(1.0);

    SUCCEED();
}

TEST_CASE("Task does not execute before interval") {
    conductor::Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Test",
        1.0,
        [&](double) {
            calls++;
        }
    );

    scheduler.update(0.5);

    REQUIRE(calls == 0);
}

TEST_CASE("Task executes after interval") {
    conductor::Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Test",
        1.0,
        [&](double) {
            calls++;
        }
    );

    scheduler.update(1.0);

    REQUIRE(calls == 1);
}

TEST_CASE("Task executes multiple times for large delta") {
    conductor::Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Physics",
        100.0,
        [&](double) {
            calls++;
        }
    );

    scheduler.update(0.05);

    REQUIRE(calls == 5);
}

TEST_CASE("Scheduler carries remainder between updates") {
    conductor::Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Physics",
        10.0,
        [&](double) {
            calls++;
        }
    );

    scheduler.update(0.05);

    REQUIRE(calls == 0);

    scheduler.update(0.05);

    REQUIRE(calls == 1);
}

TEST_CASE("Paused task does not execute") {
    conductor::Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Test",
        1.0,
        [&](double) {
            calls++;
        }
    );

    task.pause();

    scheduler.update(5.0);

    REQUIRE(calls == 0);
}

TEST_CASE("Resumed task executes") {
    conductor::Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Test",
        1.0,
        [&](double) {
            calls++;
        }
    );

    task.pause();

    scheduler.update(5.0);

    task.resume();

    scheduler.update(1.0);

    REQUIRE(calls == 1);
}

TEST_CASE("Cancelled task is never executed") {
    conductor::Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Test",
        1.0,
        [&](double) {
            calls++;
        }
    );

    task.cancel();

    scheduler.update(10.0);

    REQUIRE(calls == 0);
}

TEST_CASE("TaskHandle unregisters on destruction") {
    conductor::Scheduler scheduler;

    int calls = 0;

    {
        auto task = scheduler.schedule(
            "Test",
            1.0,
            [&](double) {
                calls++;
            }
        );
    }

    scheduler.update(5.0);

    REQUIRE(calls == 0);
}

TEST_CASE("TaskHandle move constructor transfers ownership") {
    conductor::Scheduler scheduler;

    int calls = 0;

    auto task1 = scheduler.schedule(
        "Test",
        1.0,
        [&](double) {
            calls++;
        }
    );

    auto task2 = std::move(task1);

    scheduler.update(1.0);

    REQUIRE(calls == 1);
}

TEST_CASE("TaskHandle move assignment transfers ownership") {
    conductor::Scheduler scheduler;

    int calls = 0;

    auto task1 = scheduler.schedule(
        "Task1",
        1.0,
        [&](double) {
            calls++;
        }
    );

    auto task2 = scheduler.schedule(
        "Task2",
        10.0,
        [](double){}
    );

    task2 = std::move(task1);

    scheduler.update(1.0);

    REQUIRE(calls == 1);
}

TEST_CASE("Task frequency can be changed") {
    conductor::Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Test",
        1.0,
        [&](double) {
            calls++;
        }
    );

    task.set_frequency(2.0);

    scheduler.update(0.5);

    REQUIRE(calls == 1);
}

TEST_CASE("Multiple tasks execute independently") {
    conductor::Scheduler scheduler;

    int physics = 0;
    int render = 0;

    auto p = scheduler.schedule(
        "Physics",
        100.0,
        [&](double) {
            physics++;
        }
    );

    auto r = scheduler.schedule(
        "Render",
        10.0,
        [&](double) {
            render++;
        }
    );

    scheduler.update(0.1);

    REQUIRE(physics == 10);
    REQUIRE(render == 1);
}

TEST_CASE("Removing one task leaves remaining tasks active") {
    conductor::Scheduler scheduler;

    int a = 0;
    int b = 0;

    auto taskA = scheduler.schedule(
        "A",
        1.0,
        [&](double) {
            a++;
        }
    );

    auto taskB = scheduler.schedule(
        "B",
        1.0,
        [&](double) {
            b++;
        }
    );

    taskA.cancel();

    scheduler.update(1.0);

    REQUIRE(a == 0);
    REQUIRE(b == 1);
}

TEST_CASE("Splitting delta produces identical execution count") {
    conductor::Scheduler scheduler;

    int calls = 0;

    auto task = scheduler.schedule(
        "Physics",
        100.0,
        [&](double) {
            calls++;
        }
    );

    scheduler.update(0.025);
    scheduler.update(0.025);

    REQUIRE(calls == 5);
}