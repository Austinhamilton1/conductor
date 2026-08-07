#include "Scheduler.h"

#include <stdexcept>

conductor::Scheduler::TaskHandle conductor::Scheduler::schedule(
    std::string name,
    double frequency,
    ScheduledTask::Callback callback
) {
    if(frequency <= 0.0) {
        throw std::invalid_argument(
            "Task frequency must be greater than zero"
        );
    }
    
    double interval = 1.0 / frequency;
    TaskID id = m_next_id++;

    m_tasks.emplace(
        id,
        ScheduledTask(
            std::move(name),
            std::move(callback),
            interval
        )
    );

    return TaskHandle(
        this,
        id
    );
}

void conductor::Scheduler::update(double dt) {
    for(auto& it : m_tasks) {
        auto& task = it.second;

        task.update(dt);
    }
}

void conductor::Scheduler::remove(TaskID id) {
    auto it = m_tasks.find(id);

    if(it != m_tasks.end()) {
        m_tasks.erase(it);
    }
}

void conductor::Scheduler::pause(TaskID id) {
    auto it = m_tasks.find(id);

    if(it != m_tasks.end()) {
        it->second.pause();
    }
}

void conductor::Scheduler::resume(TaskID id) {
    auto it = m_tasks.find(id);

    if(it != m_tasks.end()) {
        it->second.resume();
    }
}

void conductor::Scheduler::set_frequency(TaskID id, double frequency) {
    auto it = m_tasks.find(id);

    if(it != m_tasks.end()) {
        it->second.set_frequency(frequency);
    }
}

conductor::Scheduler::TaskHandle::~TaskHandle() {
    cancel();
}

conductor::Scheduler::TaskHandle::TaskHandle(TaskHandle&& other) noexcept 
    : m_scheduler(other.m_scheduler),
      m_id(other.m_id)
{
    other.m_scheduler = nullptr;
    other.m_id = 0;
}

conductor::Scheduler::TaskHandle& conductor::Scheduler::TaskHandle::operator=(TaskHandle&& other) noexcept {
    if(this != &other) {
        cancel();

        m_scheduler = other.m_scheduler;
        m_id = other.m_id;

        other.m_scheduler = nullptr;
        other.m_id = 0;
    }

    return *this;
}

bool conductor::Scheduler::TaskHandle::valid() const {
    return m_scheduler != nullptr;
}

void conductor::Scheduler::TaskHandle::cancel() {
    if(!m_scheduler)
        return;

    m_scheduler->remove(m_id);

    m_scheduler = nullptr;
    m_id = 0;
}

void conductor::Scheduler::TaskHandle::pause() {
    if(m_scheduler)
        m_scheduler->pause(m_id);
}

void conductor::Scheduler::TaskHandle::resume() {
    if(m_scheduler)
        m_scheduler->resume(m_id);
}

void conductor::Scheduler::TaskHandle::set_frequency(double frequency) {
    if(m_scheduler)
        m_scheduler->set_frequency(m_id, frequency);
}