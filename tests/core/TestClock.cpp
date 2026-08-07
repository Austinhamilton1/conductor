#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <chrono>

#include "core/Clock.h"

TEST_CASE("Clock moves forward") {
    auto a = conductor::Clock::now();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto b = conductor::Clock::now();

    REQUIRE(b > a);
}

TEST_CASE("Ticking clock moves clock forward") {
    conductor::Clock c;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    c.tick();
    
    REQUIRE(c.delta_seconds() > 0.0);
    REQUIRE(c.total_seconds() > 0.0);
}