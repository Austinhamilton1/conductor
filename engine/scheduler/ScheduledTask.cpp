#include "ScheduledTask.h"

#include <stdexcept>

namespace conductor {
    constexpr double EPSILON = 1e-9;

    void ScheduledTask::advance(double dt) {
        if(m_paused)
            return;

        m_accumulator += dt;
    }

    bool ScheduledTask::due() const {
        return !m_paused && m_accumulator + EPSILON >= m_interval;
    }

    void ScheduledTask::execute() {
        if(!due())
            return;

        m_state = TaskState::Running;

        try {
            m_callback(m_interval);

            m_accumulator -= m_interval;

            if(m_accumulator < EPSILON)
                m_accumulator = 0.0;

            m_generation++;

            m_state = TaskState::Waiting;
        } catch(...) {
            m_state = TaskState::Failed;
            throw;
        }
    }

    void ScheduledTask::pause() {
        m_paused = true;
    }

    void ScheduledTask::resume() {
        m_paused = false;
    }

    bool ScheduledTask::paused() const {
        return m_paused;
    }

    void ScheduledTask::set_frequency(double frequency) {
        if(frequency <= 0.0) {
            throw std::invalid_argument(
                "Task frequency must be greater than zero"
            );
        }

        m_interval = 1.0 / frequency;
    }

    double ScheduledTask::frequency() const {
        return 1.0 / m_interval;
    }

    const std::string& ScheduledTask::name() const {
        return m_name;
    }

    uint64_t ScheduledTask::generation() const {
        return m_generation;
    }

    TaskState ScheduledTask::state() const {
        return m_state;
    }

    void ScheduledTask::set_state(TaskState state) {
        m_state = state;
    }
}