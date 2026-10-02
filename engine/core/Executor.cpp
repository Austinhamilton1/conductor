#include "Executor.h"

#include <stdexcept>
#include <utility>

#include "taskflow/taskflow.hpp"

namespace conductor {
    class Executor::Impl {
    public:
        explicit Impl(std::size_t worker_count)
            : executor(worker_count) {};

        tf::Executor executor;
    };

    Executor::Executor(std::size_t worker_count)
        : m_impl(new Impl(worker_count)) {}

    Executor::~Executor() {
        delete m_impl;
    }

    Executor::Executor(Executor&& other) noexcept
        : m_impl(std::exchange(other.m_impl, nullptr)) {}

    Executor& Executor::operator=(Executor&& other) noexcept {
        if(this != &other) {
            delete m_impl;

            m_impl = std::exchange(other.m_impl, nullptr);
        }

        return *this;
    }

    void Executor::parallel(
        std::vector<std::function<void()>> tasks
    ) {
        if(tasks.empty())
            return;

        if(m_impl == nullptr) {
            throw std::logic_error(
                "Cannot used a moved-from Executor"
            );
        }

        tf::Taskflow taskflow;

        for(auto& task : tasks) {
            taskflow.emplace(
                std::move(task)
            );
        }

        m_impl->executor
            .run(taskflow)
            .wait();
    }

    std::size_t Executor::worker_count() const noexcept {
        if(m_impl == nullptr)
            return 0;

        return m_impl->executor.num_workers();
    }
}