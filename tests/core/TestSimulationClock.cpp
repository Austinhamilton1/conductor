#include <catch2/catch_test_macros.hpp>

#include "core/SimulationClock.h"

TEST_CASE("Clock update updates correctly", "[core]") {
    conductor::SimulationClock c;

    c.update(20.0);
    
    REQUIRE(c.delta_seconds() == 20.0);
    REQUIRE(c.total_seconds() == 20.0);
}

TEST_CASE("Pausing clock disables update", "[core]") {
    conductor::SimulationClock c;

    c.update(20.0);

    c.pause();

    c.update(20.0);
    REQUIRE(c.delta_seconds() == 0.0);
    REQUIRE(c.total_seconds() == 20.0);

    c.resume();

    c.update(20.0);
    
    REQUIRE(c.delta_seconds() == 20.0);
    REQUIRE(c.total_seconds() == 40.0);
}

TEST_CASE("Clock scales update deltas", "[core]") {
    conductor::SimulationClock c;

    c.set_time_scale(2.0);

    c.update(5.0);

    REQUIRE(c.delta_seconds() == 10.0);
    REQUIRE(c.total_seconds() == 10.0);
}