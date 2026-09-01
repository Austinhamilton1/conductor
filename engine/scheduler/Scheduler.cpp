#include "Scheduler.h"

#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <iostream>

namespace conductor {
    DependencyResult Scheduler::depends_on(SystemId child, SystemId parent) {
        auto child_it = m_systems.find(child);
        auto parent_it = m_systems.find(parent);

        if(child_it == m_systems.end())
            return DependencyResult::InvalidChild;

        if(parent_it == m_systems.end())
            return DependencyResult::InvalidParent;

        if(child == parent)
            return DependencyResult::SelfDependency;

        if(has_dependency(child, parent))
            return DependencyResult::DuplicateDependency;

        if(creates_cycle(child, parent))
            return DependencyResult::CycleDetected;

        child_it->second.dependencies.push_back(
            SystemDependency{
                .parent = parent,
                .consumed_generation = 
                    parent_it->second.schedule.generation()
            }
        );

        parent_it->second.dependents.push_back(child);

        return DependencyResult::Success;
    }

    bool Scheduler::dependencies_ready(SystemId id) const {
        auto it = m_systems.find(id);

        if(it == m_systems.end())
            return false;

        const auto& node = it->second;

        for(const auto& dependency : node.dependencies) {
            auto parent_it = m_systems.find(dependency.parent);

            if(parent_it == m_systems.end())
                return false;

            const auto parent_generation = 
                parent_it->second.schedule.generation();

            if(parent_generation <= dependency.consumed_generation)
                return false;
        }

        return true;
    }

    void Scheduler::consume_dependencies(SystemId id) {
        auto it = m_systems.find(id);

        if(it == m_systems.end())
            return;

        auto& node = it->second;

        for(auto& dependency : node.dependencies) {
            auto parent_it = m_systems.find(dependency.parent);

            if(parent_it == m_systems.end())
                continue;

            dependency.consumed_generation = 
                parent_it->second.schedule.generation();
        }
    }

    bool Scheduler::creates_cycle(
        SystemId child,
        SystemId parent
    ) const {
        std::vector<SystemId> stack;
        std::unordered_set<SystemId> visited;

        stack.push_back(parent);

        while(!stack.empty()) {
            const SystemId current = stack.back();
            stack.pop_back();

            if(!visited.insert(current).second)
                continue;

            if(current == child)
                return true;

            auto it = m_systems.find(current);

            if(it == m_systems.end())
                continue;

            for(const auto dependency : it->second.dependencies) {
                stack.push_back(dependency.parent);
            }
        }

        return false;
    }

    bool Scheduler::has_dependency(
        SystemId child,
        SystemId parent
    ) const {
        auto it = m_systems.find(child);

        if(it == m_systems.end())
            return false;

        for(const auto& dependency : it->second.dependencies) {
            if(dependency.parent == parent)
                return true;
        }

        return false;
    }

    void Scheduler::update(double delta) {
        // Advance the timing state of every task
        for(auto& [id, node] : m_systems) {
            node.schedule.advance(delta);
        }

        bool progress = true;

        while(progress) {
            progress = false;

            for(auto& [id, node] : m_systems) {
                if(!node.schedule.due())
                    continue;

                if(node.schedule.state() == SystemState::Running)
                    continue;

                if(node.schedule.state() == SystemState::Failed)
                    continue;

                if(!dependencies_ready(id))
                    continue;

                if(execute_system(id))
                    progress = true;
            }
        }
    }

    bool Scheduler::execute_system(SystemId id) {
        auto it = m_systems.find(id);

        if(it == m_systems.end())
            return false;

        auto& node = it->second;

        if(!node.schedule.due())
            return false;

        if(!dependencies_ready(id))
            return false;

        node.schedule.set_state(SystemState::Running);

        try {
            node.executor->execute(
                m_commands,
                m_clock
            );

            node.schedule.consume_interval();

            node.schedule.increment_generation();

            node.schedule.set_state(SystemState::Waiting);

            consume_dependencies(id);

            return true;
        } catch(...) {
            node.schedule.set_state(SystemState::Failed);

            throw;
        }

        return true;
    }

    void Scheduler::remove(SystemId id) {
        auto it = m_systems.find(id);

        if(it == m_systems.end())
            return;

        // remove this task from all parent dependent lists
        for(const auto& dependency : it->second.dependencies) {
            auto parent_it = m_systems.find(dependency.parent);

            if(parent_it == m_systems.end())
                continue;

            auto& dependents = parent_it->second.dependents;

            std::erase(dependents, id);
        }

        // Remove this task from all child dependency lists
        for(const SystemId dependent : it->second.dependents) {
            auto child_it = m_systems.find(dependent);

            if(child_it == m_systems.end())
                continue;

            auto& dependencies = child_it->second.dependencies;

            std::erase_if(
                dependencies,
                [id](const SystemDependency& dependency) {
                    return dependency.parent == id;
                }
            );
        }

        m_systems.erase(it);
    }

    void Scheduler::pause(SystemId id) {
        auto it = m_systems.find(id);

        if(it != m_systems.end()) {
            it->second.schedule.pause();
        }
    }

    void Scheduler::resume(SystemId id) {
        auto it = m_systems.find(id);

        if(it != m_systems.end()) {
            it->second.schedule.resume();
        }
    }

    void Scheduler::set_frequency(SystemId id, double frequency) {
        auto it = m_systems.find(id);

        if(it != m_systems.end()) {
            it->second.schedule.set_frequency(frequency);
        }
    }

    Scheduler::SystemHandle::~SystemHandle() {
        cancel();
    }

    Scheduler::SystemHandle::SystemHandle(SystemHandle&& other) noexcept 
        : m_scheduler(other.m_scheduler),
          m_id(other.m_id)
    {
        other.m_scheduler = nullptr;
        other.m_id = 0;
    }

    Scheduler::SystemHandle&
    Scheduler::SystemHandle::operator=(SystemHandle&& other) noexcept {
        if(this != &other) {
            cancel();

            m_scheduler = other.m_scheduler;
            m_id = other.m_id;

            other.m_scheduler = nullptr;
            other.m_id = 0;
        }

        return *this;
    }

    SystemId Scheduler::SystemHandle::id() const {
        return m_id;
    }

    uint32_t Scheduler::SystemHandle::generation() const {
        if(!m_scheduler)
            return 0;

        auto it = m_scheduler->m_systems.find(m_id);

        if(it == m_scheduler->m_systems.end())
            return 0;

        return it->second.schedule.generation();
    }

    SystemState Scheduler::SystemHandle::state() const {
        if(!m_scheduler)
            return SystemState::Failed;

        auto it = m_scheduler->m_systems.find(m_id);

        if(it == m_scheduler->m_systems.end())
            return SystemState::Failed;

        return it->second.schedule.state();
    }

    bool Scheduler::SystemHandle::paused() const {
        if(!m_scheduler)
            return false;

        auto it = m_scheduler->m_systems.find(m_id);

        if(it == m_scheduler->m_systems.end())
            return false;

        return it->second.schedule.paused();
    }

    DependencyResult Scheduler::SystemHandle::depends_on(const SystemHandle& parent) {
        if(!m_scheduler)
            return DependencyResult::InvalidChild;

        if(!parent.m_scheduler)
            return DependencyResult::InvalidParent;

        if(m_scheduler != parent.m_scheduler) 
            return DependencyResult::InvalidParent;

        return m_scheduler->depends_on(m_id, parent.m_id);
    }

    bool Scheduler::SystemHandle::valid() const {
        return m_scheduler != nullptr;
    }

    void Scheduler::SystemHandle::cancel() {
        if(!m_scheduler)
            return;

        m_scheduler->remove(m_id);

        m_scheduler = nullptr;
        m_id = 0;
    }

    void Scheduler::SystemHandle::pause() {
        if(m_scheduler)
            m_scheduler->pause(m_id);
    }

    void Scheduler::SystemHandle::resume() {
        if(m_scheduler)
            m_scheduler->resume(m_id);
    }

    void Scheduler::SystemHandle::set_frequency(double frequency) {
        if(m_scheduler)
            m_scheduler->set_frequency(m_id, frequency);
    }
}