#include "SystemSchedule.h"

#include <stdexcept>

namespace conductor {
    constexpr double EPSILON = 1e-9;

    void SystemSchedule::advance(double dt) {
        if(m_paused)
            return;

        m_accumulator += dt;
    }

    bool SystemSchedule::due() const {
        return !m_paused && m_accumulator + EPSILON >= m_interval;
    }

    void SystemSchedule::pause() {
        m_paused = true;
    }

    void SystemSchedule::resume() {
        m_paused = false;
    }

    bool SystemSchedule::paused() const {
        return m_paused;
    }

    void SystemSchedule::set_frequency(double frequency) {
        if(frequency <= 0.0) {
            throw std::invalid_argument(
                "Task frequency must be greater than zero"
            );
        }

        m_interval = 1.0 / frequency;
    }

    double SystemSchedule::frequency() const {
        return 1.0 / m_interval;
    }

    const std::string& SystemSchedule::name() const {
        return m_name;
    }

    uint64_t SystemSchedule::generation() const {
        return m_generation;
    }

    SystemState SystemSchedule::state() const {
        return m_state;
    }

    void SystemSchedule::set_state(SystemState state) {
        m_state = state;
    }

    void SystemSchedule::consume_interval() noexcept {
        m_accumulator -= m_interval;

        if(m_accumulator < EPSILON)
            m_accumulator = 0.0;
    }

    void SystemSchedule::increment_generation() noexcept {
        ++m_generation;
    }
}