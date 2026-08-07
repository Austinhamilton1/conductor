#pragma once

#include <chrono>

namespace conductor {
    class Clock {
    private:
        std::chrono::steady_clock::time_point m_last;
        double m_delta = 0.0;
        double m_total = 0.0;

    public:
        Clock() : m_last(std::chrono::steady_clock::now()) {};
        
        static double now();

        void tick();

        double delta_seconds() const;
        double total_seconds() const;
    };
};