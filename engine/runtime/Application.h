#pragma once

#include <memory>

#include "core/Clock.h"
#include "core/SimulationClock.h"
#include "core/NonCopyable.h"
#include "events/EventBus.h"
#include "scheduler/Scheduler.h"
#include "layers/Layer.h"
#include "layers/LayerStack.h"
#include "ApplicationContext.h"

namespace conductor {
    class Application : public NonCopyable {
    private:
        bool m_running = false;
        
        Clock m_clock;
        SimulationClock m_simulation_clock;

        EventBus m_events;
        Scheduler m_scheduler;

        ApplicationContext m_context {
            m_clock,
            m_simulation_clock,
            m_events,
            m_scheduler
        };

        LayerStack m_layers;

    protected:
        virtual void initialize() {};
        virtual void update(double) {};
        virtual void shutdown() {};

    public:
        Application() : m_layers(m_context) {};
        virtual ~Application() = default;

        Clock& clock();
        SimulationClock& simulation_clock();

        EventBus& events();
        Scheduler& scheduler();

        void run();
        void stop();

        void add_layer(std::unique_ptr<Layer> layer);
        void pop_layer();
    };
};