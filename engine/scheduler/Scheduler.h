#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include <vector>

#include "DependencyResult.h"
#include "TaskId.h"
#include "ScheduledTask.h"
#include "TaskDependency.h"
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

            TaskId id() const;

            DependencyResult depends_on(TaskHandle parent);
        };

        TaskHandle schedule(
            std::string name,
            double frequency,
            ScheduledTask::Callback callback
        );

        DependencyResult depends_on(
            TaskId child,
            TaskId parent
        );

        void update(double delta);

    private:
        struct TaskNode {
            ScheduledTask task;

            std::vector<TaskDependency> dependencies;
            std::vector<TaskId> dependents;

            TaskNode(
                std::string name,
                ScheduledTask::Callback callback,
                double interval
            )
                : task(
                    std::move(name),
                    std::move(callback),
                    interval
                ) {}
        };

        TaskId m_next_id = 1;
        
        std::unordered_map<TaskId, TaskNode> m_tasks;

        void remove(TaskId id);
        void pause(TaskId id);
        void resume(TaskId id);

        void set_frequency(
            TaskId,
            double frequency
        );

        bool dependencies_ready(TaskId id) const;

        bool has_dependency(
            TaskId child,
            TaskId parent
        ) const;

        bool creates_cycle(
            TaskId child,
            TaskId parent
        ) const;

        void consume_dependencies(TaskId id);

        bool execute_task(TaskId id);
    };
};