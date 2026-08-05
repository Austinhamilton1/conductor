#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <chrono>

#include "Clock.h"

TEST_CASE("Clock moves forward") {
    auto a = conductor::Clock::now();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto b = conductor::Clock::now();

    REQUIRE(b > a);
}