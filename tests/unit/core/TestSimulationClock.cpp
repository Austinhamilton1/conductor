#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/SimulationClock.h"

namespace {
    constexpr double hz = 1.0 / 60.0 + 0.000001;
}

TEST_CASE("Clock update updates correctly", "[core]") {
    conductor::SimulationClock c;

    c.update(20.0);

    while(c.step_available()) {
        c.consume_step();
    }
    
    REQUIRE_THAT(c.total_seconds(), Catch::Matchers::WithinAbs(20.0, hz));
}

TEST_CASE("Pausing clock disables update", "[core]") {
    conductor::SimulationClock c;

    c.update(20.0);

    while(c.step_available()) {
        c.consume_step();
    }

    c.pause();

    c.update(20.0);

    while(c.step_available()) {
        c.consume_step();
    }

    REQUIRE_THAT(c.total_seconds(), Catch::Matchers::WithinAbs(20.0, hz));

    c.resume();

    c.update(20.0);

    while(c.step_available()) {
        c.consume_step();
    }
    
    REQUIRE_THAT(c.total_seconds(), Catch::Matchers::WithinAbs(40.0, hz));
}

TEST_CASE("Clock scales update deltas", "[core]") {
    conductor::SimulationClock c;

    c.set_time_scale(2.0);

    c.update(5.0);

    while(c.step_available()) {
        c.consume_step();
    }

    REQUIRE_THAT(c.total_seconds(), Catch::Matchers::WithinAbs(10.0, hz));
}