#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include "SystemState.h"

namespace conductor {
    class SystemSchedule {
    public:
        SystemSchedule(
            std::string name,
            double interval
        ) : m_name(std::move(name)),
            m_interval(interval) {}

        void advance(double dt);

        bool due() const;

        void pause();
        void resume();

        bool paused() const;

        void set_frequency(double frequency);

        double frequency() const;

        const std::string& name() const;

        uint64_t generation() const;

        SystemState state() const;

        void set_state(SystemState state);

        void consume_interval() noexcept;

        void increment_generation() noexcept;

    private:
        std::string m_name;

        bool m_paused = false;

        double m_interval;
        double m_accumulator = 0.0;

        uint64_t m_generation = 0;
        
        SystemState m_state = SystemState::Waiting;
    };
}