#include "SimulationClock.h"

namespace conductor {
    void SimulationClock::update(double real_delta) {
        if(m_paused) {
            m_delta = 0.0;
            return;
        }

        m_delta = real_delta * m_time_scale;
        m_total += m_delta;
    }

    void SimulationClock::pause() {
        m_paused = true;
    }

    void SimulationClock::resume() {
        m_paused = false;
    }

    void SimulationClock::set_time_scale(double scale) {
        if(scale < 0.0)
            return;
            
        m_time_scale = scale;
    }

    double SimulationClock::delta_seconds() const {
        return m_delta;
    }

    double SimulationClock::total_seconds() const {
        return m_total;
    }
}