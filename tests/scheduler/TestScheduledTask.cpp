#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "scheduler/ScheduledTask.h"

TEST_CASE("ScheduledTask executes at its configured frequency") {
    int calls = 0;

    conductor::ScheduledTask task(
        "Test",
        [&](double) {
            ++calls;
        },
        0.1
    );

    task.advance(0.05);

    REQUIRE(calls == 0);
    REQUIRE_FALSE(task.due());

    task.advance(0.05);

    REQUIRE(task.due());

    task.execute();

    REQUIRE(calls == 1);
    REQUIRE(task.generation() == 1);
}

TEST_CASE("ScheduledTask does not execute before its interval") {
    int calls = 0;

    conductor::ScheduledTask task(
        "Test",
        [&](double) {
            ++calls;
        },
        0.1
    );

    task.advance(0.09);

    REQUIRE_FALSE(task.due());

    task.execute();

    REQUIRE(calls == 0);
    REQUIRE(task.generation() == 0);
}

TEST_CASE("ScheduledTask accumulates elapsed time") {
    int calls = 0;

    conductor::ScheduledTask task(
        "Test",
        [&](double) {
            ++calls;
        },
        0.1
    );

    task.advance(0.06);
    REQUIRE_FALSE(task.due());

    task.advance(0.06);
    REQUIRE(task.due());

    task.execute();

    REQUIRE(calls == 1);
    REQUIRE(task.generation() == 1);
}

TEST_CASE("ScheduledTask preserves excess accumulated time") {
    int calls = 0;

    conductor::ScheduledTask task(
        "Test",
        [&](double) {
            ++calls;
        },
        0.1
    );

    task.advance(0.25);

    REQUIRE(task.due());

    task.execute();

    REQUIRE(calls == 1);
    REQUIRE(task.generation() == 1);

    // 0.15 seconds remain accumulated,
    // so another execution is immediately due.
    REQUIRE(task.due());

    task.execute();

    REQUIRE(calls == 2);
    REQUIRE(task.generation() == 2);
}

TEST_CASE("ScheduledTask increments generation after execution") {
    conductor::ScheduledTask task(
        "Test",
        [](double) {},
        0.1
    );

    REQUIRE(task.generation() == 0);

    task.advance(0.1);
    task.execute();

    REQUIRE(task.generation() == 1);

    task.advance(0.1);
    task.execute();

    REQUIRE(task.generation() == 2);
}

TEST_CASE("ScheduledTask can be paused") {
    int calls = 0;

    conductor::ScheduledTask task(
        "Test",
        [&](double) {
            ++calls;
        },
        0.1
    );

    task.pause();

    task.advance(1.0);

    REQUIRE_FALSE(task.due());

    task.execute();

    REQUIRE(calls == 0);
    REQUIRE(task.generation() == 0);
}

TEST_CASE("ScheduledTask resumes after being paused") {
    int calls = 0;

    conductor::ScheduledTask task(
        "Test",
        [&](double) {
            ++calls;
        },
        0.1
    );

    task.pause();
    task.advance(1.0);

    task.resume();

    task.advance(0.1);

    REQUIRE(task.due());

    task.execute();

    REQUIRE(calls == 1);
}

TEST_CASE("ScheduledTask frequency can be changed") {
    conductor::ScheduledTask task(
        "Test",
        [](double) {},
        0.1
    );

    REQUIRE_THAT(task.frequency(), Catch::Matchers::WithinAbs(10.0, 0.0001));

    task.set_frequency(20.0);

    REQUIRE_THAT(task.frequency(), Catch::Matchers::WithinAbs(20.0, 0.0001));
}

TEST_CASE("ScheduledTask rejects invalid frequency") {
    conductor::ScheduledTask task(
        "Test",
        [](double) {},
        0.1
    );

    REQUIRE_THROWS(task.set_frequency(0.0));
    REQUIRE_THROWS(task.set_frequency(-10.0));
}

TEST_CASE("ScheduledTask callback receives its interval") {
    double received_dt = 0.0;

    conductor::ScheduledTask task(
        "Test",
        [&](double dt) {
            received_dt = dt;
        },
        0.1
    );

    task.advance(0.1);
    task.execute();

    REQUIRE_THAT(received_dt, Catch::Matchers::WithinAbs(0.1, 0.0001));
}

TEST_CASE("ScheduledTask enters failed state when callback throws") {
    conductor::ScheduledTask task(
        "Test",
        [](double) {
            throw std::runtime_error("failure");
        },
        0.1
    );

    task.advance(0.1);

    REQUIRE_THROWS(task.execute());

    REQUIRE(
        task.state() == conductor::TaskState::Failed
    );

    // Failed task must not produce a generation.
    REQUIRE(task.generation() == 0);
}