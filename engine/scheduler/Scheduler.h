#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>

#include "ScheduledTask.h"
#include "core/NonCopyable.h"

namespace conductor {    
    class Scheduler {
    public:
        class TaskHandle : public NonCopyable {
        private:

            Scheduler *m_scheduler = nullptr;
            uint64_t m_id = 0;
        
        public:
            TaskHandle() = default;
            TaskHandle(Scheduler *scheduler, uint64_t id)
                : m_scheduler(scheduler),
                m_id(id) {};
            ~TaskHandle();

            TaskHandle(TaskHandle&& other) noexcept;
            TaskHandle& operator=(TaskHandle&& other) noexcept;

            void cancel();
            void pause();
            void resume();

            void set_frequency(double frequency);
            bool valid() const;
        };

        using TaskID = uint64_t;

        TaskHandle schedule(
            std::string name,
            double frequency,
            ScheduledTask::Callback callback
        );

        void update(double delta);

    private:
        TaskID m_next_id = 1;

        std::unordered_map<TaskID, ScheduledTask> m_tasks;

        void remove(TaskID id);
        void pause(TaskID id);
        void resume(TaskID id);

        void set_frequency(TaskID id, double frequency);
    };
};