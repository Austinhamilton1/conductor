#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <chrono>

#include "core/Timer.h"

TEST_CASE("Timer measures elapsed time") {
    conductor::Timer timer;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    REQUIRE(timer.milliseconds() >= 100);
}