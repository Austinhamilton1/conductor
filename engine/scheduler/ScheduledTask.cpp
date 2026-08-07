#include "ScheduledTask.h"

constexpr double EPSILON = 1e-9;

void conductor::ScheduledTask::update(double dt) {
    if(m_paused)
        return;

    m_accumulator += dt;

    while(m_accumulator + EPSILON >= m_interval) {
        m_callback(m_interval);
        m_accumulator -= m_interval;

        if(m_accumulator < EPSILON) {
            m_accumulator = 0.0;
        }
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
    if(frequency > 0.0)
        m_interval = 1.0 / frequency;
}

double conductor::ScheduledTask::frequency() const {
    return 1.0 / m_interval;
}

const std::string& conductor::ScheduledTask::name() const {
    return m_name;
}