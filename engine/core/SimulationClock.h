#pragma once

namespace conductor {
    class SimulationClock {
    public:
        SimulationClock(double timestep = 1.0 / 60.0)
            : m_timestep(timestep) {}

        void update(double real_delta);

        void pause();
        void resume();

        void set_time_scale(double scale);

        double delta_seconds() const;
        double total_seconds() const;

        bool step_available() const;
        double consume_step();
        
    private:
        bool m_paused = false;

        double m_time_scale = 1.0;
        double m_timestep = 1.0 / 60.0;

        double m_accumulator = 0.0;
        double m_delta = 0.0;
        double m_total = 0.0;
    };
}
