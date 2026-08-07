#include "ScheduledTask.h"

#include <stdexcept>

constexpr double EPSILON = 1e-9;

void conductor::ScheduledTask::advance(double dt) {
    if(m_paused)
        return;

    m_accumulator += dt;
}

bool conductor::ScheduledTask::due() const {
    return !m_paused && m_accumulator + EPSILON >= m_interval;
}

void conductor::ScheduledTask::execute() {
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

void conductor::ScheduledTask::pause() {
    m_paused = true;
}

void conductor::ScheduledTask::resume() {
    m_paused = false;
}

bool conductor::ScheduledTask::paused() const {
    return m_paused;
}

void conductor::ScheduledTask::set_frequency(double frequency) {
    if(frequency <= 0.0) {
        throw std::invalid_argument(
            "Task frequency must be greater than zero"
        );
    }

    m_interval = 1.0 / frequency;
}

double conductor::ScheduledTask::frequency() const {
    return 1.0 / m_interval;
}

const std::string& conductor::ScheduledTask::name() const {
    return m_name;
}

uint64_t conductor::ScheduledTask::generation() const {
    return m_generation;
}

conductor::TaskState conductor::ScheduledTask::state() const {
    return m_state;
}

void conductor::ScheduledTask::set_state(TaskState state) {
    m_state = state;
}