#pragma once

#include <chrono>

namespace conductor {
    class Timer {
    public:
        Timer()
            : start(std::chrono::steady_clock::now()) {};

        double seconds();
        double milliseconds();
        double microseconds();
        
    private:
        std::chrono::time_point<std::chrono::steady_clock> start;
    };
}