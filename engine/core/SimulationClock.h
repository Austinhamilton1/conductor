#pragma once

namespace conductor {
    class SimulationClock {
    public:
        SimulationClock()
            : m_paused(false),
              m_time_scale(1.0),
              m_delta(0.0),
              m_total(0.0) {}

        void update(double real_delta);

        void pause();
        void resume();

        void set_time_scale(double scale);

        double delta_seconds() const;
        double total_seconds() const;
        
    private:
        bool m_paused;
        double m_time_scale;
        double m_delta;
        double m_total;
    };
}
