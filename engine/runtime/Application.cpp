#include "Application.h"

#include <utility>

conductor::Clock& conductor::Application::clock() {
    return m_clock;
}

conductor::SimulationClock& conductor::Application::simulation_clock() {
    return m_simulation_clock;
}

void conductor::Application::run() {
    initialize();

    m_running = true;

    while(m_running) {
        // Update real time
        m_clock.tick();

        // Update simulation time
        m_simulation_clock.update(m_clock.delta_seconds());

        // Process events
        m_events.process();

        // Run scheduled tasks
        m_scheduler.update(m_simulation_clock.delta_seconds());

        // User update
        update(m_simulation_clock.delta_seconds());
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