#include "Application.h"
#include "ApplicationContext.h"

#include <utility>

#include <iostream>

namespace conductor {
    Application::Application(ApplicationConfig config)
        : m_duration(config.duration),
          m_step_size(config.step_size),
          m_rand(config.seed),
          m_world(m_rand)
    {
        m_simulation_clock.set_time_scale(config.time_scale);
    }

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

    Random& Application::random() {
        return m_rand;
    }

    void Application::load_plugin(const std::string& plugin_path) {
        m_plugins.load(plugin_path, m_world, m_events);
    }

    void Application::run() {
        m_running = true;
        
        m_rand.seed(m_rand.get_seed());
        initialize();

        m_world.finalize();

        ApplicationContext ctx{
            .random = m_rand,
            .clock = m_simulation_clock,
            .events = m_events,
        };

        while(m_running) {
            m_clock.tick();
            
            m_simulation_clock.update(
                m_clock.delta_seconds()
            );
            
            while(m_simulation_clock.step_available() && m_running) {
                const double dt =
                    m_simulation_clock.consume_step();

                // Process events from last frame
                m_events.process();

                // Application manages global update
                update(dt);

                // Update simulation world (ECS)
                m_world.update(dt, ctx);

                if(m_simulation_clock.total_seconds() >= m_duration) {
                    stop();
                }
            }
        }

        m_plugins.shutdown();
        shutdown();
    }

    void Application::stop() {
        m_running = false;
    }
}