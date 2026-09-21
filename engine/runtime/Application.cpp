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

    PluginManager& Application::plugins() {
        return m_plugins;
    }

    World& Application::world() {
        return m_world;
    }

    void Application::run() {
        m_running = true;
        
        initialize();

        m_world.finalize();

        while(m_running) {
            m_clock.tick();
            const double real_dt = m_clock.delta_seconds();

            m_simulation_clock.update(real_dt);
            const double simulation_dt = m_simulation_clock.delta_seconds();

            // Process events from last frame
            m_events.process();

            // Application manages global updates
            update(simulation_dt);

            // Update simulation world (ECS)
            m_world.update(simulation_dt);
        }

        m_plugins.shutdown(m_world);
        shutdown();
    }

    void Application::stop() {
        m_running = false;
    }
}