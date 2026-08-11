#include "Timer.h"

namespace conductor {
    double Timer::seconds() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(now-start).count();
    }

    double Timer::milliseconds() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(now-start).count() * 1000;
    }

    double Timer::microseconds() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(now-start).count() * 1000000;
    }
}