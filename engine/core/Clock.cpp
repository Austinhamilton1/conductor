#include "Clock.h"

#include <chrono>

double conductor::Clock::now() {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> seconds = now.time_since_epoch();
    return seconds.count();
}