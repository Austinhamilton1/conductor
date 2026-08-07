#include "Clock.h"

#include <chrono>

double conductor::Clock::now() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(
        now.time_since_epoch()
    ).count();
}

void conductor::Clock::tick() {
    auto now = std::chrono::steady_clock::now();
    
    m_delta = std::chrono::duration<double>(
        now - m_last
    ).count();
    
    m_total += m_delta;
    m_last = now;
}

double conductor::Clock::delta_seconds() const {
    return m_delta;
}

double conductor::Clock::total_seconds() const {
    return m_total;
}