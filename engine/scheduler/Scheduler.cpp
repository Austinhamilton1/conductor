#include "Scheduler.h"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>
#include <tuple>
#include <algorithm>

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
    TaskId id = m_next_id++;

    m_tasks.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple(
            std::move(name),
            std::move(callback),
            interval
        )
    );

    return TaskHandle(this, id);
}

conductor::DependencyResult conductor::Scheduler::depends_on(TaskId child, TaskId parent) {
    auto child_it = m_tasks.find(child);
    auto parent_it = m_tasks.find(parent);

    if(child_it == m_tasks.end())
        return DependencyResult::InvalidChild;

    if(parent_it == m_tasks.end())
        return DependencyResult::InvalidParent;

    if(child == parent)
        return DependencyResult::SelfDependency;

    if(has_dependency(child, parent))
        return DependencyResult::DuplicateDependency;

    if(creates_cycle(child, parent))
        return DependencyResult::CycleDetected;

    child_it->second.dependencies.push_back(
        TaskDependency {
            .parent = parent,
            .consumed_generation = parent_it->second.task.generation()
        }
    );

    parent_it->second.dependents.push_back(child);

    return DependencyResult::Success;
}

bool conductor::Scheduler::dependencies_ready(TaskId id) const {
    auto it = m_tasks.find(id);

    if(it == m_tasks.end())
        return false;

    const auto& node = it->second;

    for(const auto& dependency : node.dependencies) {
        auto parent_it = m_tasks.find(dependency.parent);

        if(parent_it == m_tasks.end())
            return false;

        const auto parent_generation = parent_it->second.task.generation();

        if(parent_generation <= dependency.consumed_generation)
            return false;
    }

    return true;
}

void conductor::Scheduler::consume_dependencies(TaskId id) {
    auto it = m_tasks.find(id);

    if(it == m_tasks.end())
        return;

    auto& node = it->second;

    for(auto& dependency : node.dependencies) {
        auto parent_it = m_tasks.find(dependency.parent);

        if(parent_it == m_tasks.end())
            continue;

        dependency.consumed_generation = parent_it->second.task.generation();
    }
}

bool conductor::Scheduler::creates_cycle(
    TaskId child,
    TaskId parent
) const {
    std::vector<TaskId> stack;
    std::unordered_set<TaskId> visited;

    stack.push_back(child);

    while(!stack.empty()) {
        const TaskId current = stack.back();
        stack.pop_back();

        if(!visited.insert(current).second)
            continue;

        if(current == parent)
            return true;

        auto it = m_tasks.find(current);

        if(it == m_tasks.end())
            continue;

        for(const auto dependency : it->second.dependencies) {
            stack.push_back(dependency.parent);
        }
    }

    return false;
}

bool conductor::Scheduler::has_dependency(
    TaskId child,
    TaskId parent
) const {
    auto it = m_tasks.find(child);

    if(it == m_tasks.end())
        return false;

    for(const auto& dependency : it->second.dependencies) {
        if(dependency.parent == parent)
            return true;
    }

    return false;
}

void conductor::Scheduler::update(double delta) {
    // Advance the timing state of every task
    for(auto& [id, node] : m_tasks) {
        node.task.advance(delta);
    }

    bool progress = true;

    while(progress) {
        progress = false;

        for(auto& [id, node] : m_tasks) {
            if(!node.task.due())
                continue;

            if(node.task.state() == TaskState::Running)
                continue;

            if(node.task.state() == TaskState::Failed)
                continue;

            if(!dependencies_ready(id))
                continue;

            if(execute_task(id))
                progress = true;
        }
    }
}

bool conductor::Scheduler::execute_task(TaskId id) {
    auto it = m_tasks.find(id);

    if(it == m_tasks.end())
        return false;

    auto& node = it->second;

    if(!node.task.due())
        return false;

    if(!dependencies_ready(id))
        return false;

    node.task.execute();

    if(node.task.state() == TaskState::Failed)
        return false;

    consume_dependencies(id);

    return true;
}

void conductor::Scheduler::remove(TaskId id) {
    auto it = m_tasks.find(id);

    if(it == m_tasks.end())
        return;

    // remove this task from all parent dependent lists
    for(const auto& dependency : it->second.dependencies) {
        auto parent_it = m_tasks.find(dependency.parent);

        if(parent_it == m_tasks.end())
            continue;

        auto& dependents = parent_it->second.dependents;

        std::erase(dependents, id);
    }

    // Remove this task from all child dependency lists
    for(const TaskId dependent : it->second.dependents) {
        auto child_it = m_tasks.find(dependent);

        if(child_it == m_tasks.end())
            continue;

        auto& dependencies = child_it->second.dependencies;

        std::erase_if(
            dependencies,
            [id](const TaskDependency& dependency) {
                return dependency.parent == id;
            }
        );
    }

    m_tasks.erase(it);
}

void conductor::Scheduler::pause(TaskId id) {
    auto it = m_tasks.find(id);

    if(it != m_tasks.end()) {
        it->second.task.pause();
    }
}

void conductor::Scheduler::resume(TaskId id) {
    auto it = m_tasks.find(id);

    if(it != m_tasks.end()) {
        it->second.task.resume();
    }
}

void conductor::Scheduler::set_frequency(TaskId id, double frequency) {
    auto it = m_tasks.find(id);

    if(it != m_tasks.end()) {
        it->second.task.set_frequency(frequency);
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

conductor::TaskId conductor::Scheduler::TaskHandle::id() const {
    return m_id;
}

conductor::DependencyResult conductor::Scheduler::TaskHandle::depends_on(TaskHandle parent) {
    return m_scheduler->depends_on(m_id, parent.id());
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