#include "Application.h"

#include <utility>

namespace conductor {
    Clock& Application::clock() {
        return m_clock;
    }

    SimulationClock& Application::simulation_clock() {
        return m_simulation_clock;
    }

    EventBus& Application::events() {
        return m_events;
    }

    void Application::run() {
        initialize();

        m_running = true;

        while(m_running) {
            m_clock.tick();
            const double real_dt = m_clock.delta_seconds();

            m_simulation_clock.update(real_dt);
            const double simulation_dt = m_simulation_clock.delta_seconds();

            // Process events from last frame
            m_events.process();

            // LayerStack handles domain specific updates (e.g., Physics)
            m_layers.update(simulation_dt); 

            // Application manages global updates
            update(simulation_dt);
        }

        shutdown();
    }

    void Application::stop() {
        m_running = false;
    }

    void Application::add_layer(std::unique_ptr<Layer> layer) {
        m_layers.push(std::move(layer));
    }

    void Application::pop_layer() {
        m_layers.pop();
    }
}