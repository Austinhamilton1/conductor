#pragma once

#include <memory>

#include "ApplicationConfig.h"
#include "core/Clock.h"
#include "core/SimulationClock.h"
#include "core/NonCopyable.h"
#include "core/Random.h"
#include "events/EventBus.h"
#include "plugins/PluginManager.h"
#include "scheduler/Scheduler.h"
#include "simulation/World.h"

#include "taskflow/taskflow.hpp"

namespace conductor {
    class Application : public NonCopyable {
    protected:
        virtual void initialize() {};
        virtual void update(double) {};
        virtual void shutdown() {};

    public:
        Application(ApplicationConfig config);
        virtual ~Application() = default;

        Clock& clock();
        SimulationClock& simulation_clock();
        EventBus& events();
        World& world();
        PluginManager& plugins();
        Random& random();
        tf::Executor& executor();

        void run();
        void stop();

        void load_plugin(const std::string& plugin_path);

        ApplicationContext get_context();
        
    private:
        bool m_running = false;

        // This needs to be declared first so that World
        // is destroyed before this (plugin lifecycles)
        PluginManager m_plugins;
        
        double m_duration;
        double m_step_size;

        Random m_rand;
        World m_world;
        Clock m_clock;
        SimulationClock m_simulation_clock;
        EventBus m_events;

        tf::Executor m_executor;
    };
}