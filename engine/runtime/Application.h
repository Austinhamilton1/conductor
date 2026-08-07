#pragma once

#include <memory>

#include "core/Clock.h"
#include "core/SimulationClock.h"
#include "events/EventBus.h"
#include "scheduler/Scheduler.h"
#include "layers/Layer.h"
#include "layers/LayerStack.h"

namespace conductor {
    class Application {
    private:
        bool m_running = false;
        
        Clock m_clock;
        SimulationClock m_simulation_clock;

        EventBus m_events;
        Scheduler m_scheduler;
        LayerStack m_layers;

    protected:
        virtual void initialize() {};
        virtual void update(double) {};
        virtual void shutdown() {};

    public:
        Application() {};
        virtual ~Application() = default;

        Clock& clock();
        SimulationClock& simulation_clock();

        void run();
        void stop();

        void add_layer(std::unique_ptr<Layer> layer);
        void pop_layer();
    };
};