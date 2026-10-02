#pragma once

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>

#include "NonCopyable.h"

namespace conductor {
    class Executor : public NonCopyable {
    public:
        explicit Executor(std::size_t worker_count = std::thread::hardware_concurrency());

        ~Executor();

        Executor(Executor&&) noexcept;
        Executor& operator=(Executor&&) noexcept;

        /*
         * Execute a collection of independent tasks in parallel.
         *
         * The call blocks until all tasks have finished.
         */
        void parallel(std::vector<std::function<void()>> tasks);

        /*
         * Returns the number of worker threads used by the executor.
         */
        std::size_t worker_count() const noexcept;

    private:
        class Impl;

        Impl *m_impl = nullptr;
    };
}