#pragma once

#include <functional>
#include <utility>
#include <string>
#include <cstdint>

#include "TaskState.h"

namespace conductor {
    class ScheduledTask {
    public:
        using Callback = std::function<void(double)>;

        ScheduledTask(
            std::string name,
            Callback callback, 
            double interval
        )
        : m_name(std::move(name)),
          m_callback(std::move(callback)),
          m_interval(interval) {};

        void advance(double dt);

        bool due() const;

        void execute();

        void pause();
        void resume();

        bool paused() const;

        void set_frequency(double frequency);

        double frequency() const;

        const std::string& name() const;

        uint64_t generation() const;

        TaskState state() const;

        void set_state(TaskState state);

    private:
        std::string m_name;
        Callback m_callback;

        bool m_paused = false;

        double m_interval;
        double m_accumulator = 0.0;

        uint64_t m_generation = 0;
        
        TaskState m_state = TaskState::Waiting;
    };
}