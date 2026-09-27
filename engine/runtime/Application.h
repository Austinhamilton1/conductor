#pragma once

#include <memory>

#include "ApplicationContext.h"
#include "core/Clock.h"
#include "core/SimulationClock.h"
#include "core/NonCopyable.h"
#include "core/Random.h"
#include "events/EventBus.h"
#include "plugins/PluginManager.h"
#include "scheduler/Scheduler.h"
#include "simulation/World.h"

namespace conductor {
    class Application : public NonCopyable {
    protected:
        virtual void initialize() {};
        virtual void update(double) {};
        virtual void shutdown() {};

    public:
        Application() : m_world(m_rand) {};
        virtual ~Application() = default;

        Clock& clock();
        SimulationClock& simulation_clock();
        EventBus& events();
        World& world();
        PluginManager& plugins();
        Random& random();

        void run();
        void stop();
        
    private:
        bool m_running = false;

        // This needs to be declared first so that World
        // is destroyed before this (plugin lifecycles)
        PluginManager m_plugins;
        
        Clock m_clock;
        SimulationClock m_simulation_clock;
        EventBus m_events;
        World m_world;
        Random m_rand;
    };
}