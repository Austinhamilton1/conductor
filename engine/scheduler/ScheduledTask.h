#pragma once

#include <functional>
#include <utility>
#include <string>

namespace conductor {
    class ScheduledTask {
    private:

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

        void update(double dt);

        void pause();
        void resume();

        bool paused() const;

        void set_frequency(double frequency);
        double frequency() const;

        const std::string& name() const;

    private:
        std::string m_name;
        Callback m_callback;
        bool m_paused = false;
        double m_interval;
        double m_accumulator = 0.0;
    };
};