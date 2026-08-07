#include "Application.h"

#include <utility>

conductor::Clock& conductor::Application::clock() {
    return m_clock;
}

conductor::SimulationClock& conductor::Application::simulation_clock() {
    return m_simulation_clock;
}

conductor::EventBus& conductor::Application::events() {
    return m_events;
}

conductor::Scheduler& conductor::Application::scheduler() {
    return m_scheduler;
}

void conductor::Application::run() {
    initialize();

    m_running = true;

    while(m_running) {
        m_clock.tick();
        const double real_dt = m_clock.delta_seconds();

        m_simulation_clock.update(real_dt);
        const double simulation_dt = m_simulation_clock.delta_seconds();

        // Process events from last frame
        m_events.process();

        // Scheduler handles complex time-based tasks
        m_scheduler.update(simulation_dt);

        // LayerStack handles domain specific updates (e.g., Physics)
        m_layers.update(simulation_dt); 

        // Application manages global updates
        update(simulation_dt);
    }

    shutdown();
}

void conductor::Application::stop() {
    m_running = false;
}

void conductor::Application::add_layer(std::unique_ptr<Layer> layer) {
    m_layers.push(std::move(layer));
}

void conductor::Application::pop_layer() {
    m_layers.pop();
}