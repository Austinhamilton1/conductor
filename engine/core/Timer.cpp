#include "Timer.h"

double conductor::Timer::seconds() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now-start).count();
}

double conductor::Timer::milliseconds() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now-start).count() * 1000;
}

double conductor::Timer::microseconds() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now-start).count() * 1000000;
}