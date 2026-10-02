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

        const bool resolves_cycle =
            has_unresolved_inferred_cycle(child, parent);

        if(!resolves_cycle && creates_cycle(child, parent))
            return DependencyResult::CycleDetected;

        child_it->second.dependencies.push_back(
            SystemDependency{
                .parent = parent,
                .consumed_generation = 
                    parent_it->second.schedule.generation(),
                .inferred = false
            }
        );

        parent_it->second.dependents.push_back(child);

        if(resolves_cycle) {
            for(auto& cycle : m_inferred_cycles) {
                if(
                    (cycle.first == child && cycle.second == parent) ||
                    (cycle.first == parent && cycle.second == child)
                ) {
                    cycle.resolved = true;
                    break;
                }
            }
        }

        return DependencyResult::Success;
    }

    DependencyResult Scheduler::add_inferred_dependency_edge(
        SystemId child,
        SystemId parent
    ) {
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
                    parent_it->second.schedule.generation(),
                .inferred = true
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

    bool Scheduler::has_dependency_path(
        SystemId child,
        SystemId parent
    ) const noexcept {
        if(m_systems.find(child) == m_systems.end())
            return false;

        if(m_systems.find(parent) == m_systems.end())
            return false;

        std::unordered_set<SystemId> visited;

        std::vector<SystemId> stack;
        stack.push_back(child);

        while(!stack.empty()) {
            const SystemId current = stack.back();
            stack.pop_back();

            if(!visited.insert(current).second)
                continue;

            const auto it = m_systems.find(current);

            if(it == m_systems.end())
                continue;

            for(const auto& dependency : it->second.dependencies) {
                if(dependency.parent == parent)
                    return true;

                stack.push_back(dependency.parent);
            }
        }

        return false;
    }

    bool Scheduler::has_inferred_cycle(
        SystemId first,
        SystemId second
    ) const {
        return std::any_of(
            m_inferred_cycles.begin(),
            m_inferred_cycles.end(),
            [first, second](const InferredCycle& cycle) {
                return 
                    (cycle.first == first && cycle.second == second) ||
                    (cycle.first == second && cycle.second == first);
            }
        );
    }

    bool Scheduler::has_unresolved_inferred_cycle(
        SystemId first,
        SystemId second
    ) const {
        return std::any_of(
            m_inferred_cycles.begin(),
            m_inferred_cycles.end(),
            [first, second](const InferredCycle& cycle) {
                if(cycle.resolved)
                    return false;

                return 
                    (cycle.first == first && cycle.second == second) ||
                    (cycle.first == second && cycle.second == first);
            }
        );
    }

    bool Scheduler::is_inferred_dependency(
        SystemId child,
        SystemId parent
    ) const {
        auto it = m_systems.find(child);

        if(it == m_systems.end())
            return false;

        for(const auto& dependency : it->second.dependencies) {
            if(dependency.parent == parent)
                return dependency.inferred;
        }

        return false;
    }

    bool Scheduler::add_inferred_dependency(
        SystemId child,
        SystemId parent
    ) {
        if(child == parent)
            return false;

        if(has_dependency(child, parent))
            return false;

        const DependencyResult result = add_inferred_dependency_edge(child, parent);

        if(result == DependencyResult::Success)
            return true;

        if(result == DependencyResult::CycleDetected) {
            if(has_dependency(parent, child)) {
                // 2-Cycle, can be resolved explicitly by user
                if(!is_inferred_dependency(parent, child)) {
                    // The user explicitly established the opposite ordering.
                    // The cycle is therefore already resolved.
                    return false;
                }

                remove_dependency(parent, child);

                if(!has_inferred_cycle(child, parent)) {
                    m_inferred_cycles.push_back(
                        InferredCycle{
                            .first = child,
                            .second = parent,
                            .resolved = false
                        }
                    );
                }

                return false;
            }

            throw std::logic_error(
                "Automatic scheduler dependency inference detected cycle"
            );
        }

        throw std::logic_error(
            "Automatic scheduler dependency inference failed"
        );
    }

    void Scheduler::infer_dependency(
        SystemId first,
        SystemId second
    ) {
        auto first_it = m_systems.find(first);
        auto second_it = m_systems.find(second);

        if(first_it == m_systems.end() || second_it == m_systems.end())
            return;

        const auto& first_accesses = first_it->second.accesses;
        const auto& second_accesses = second_it->second.accesses;

        for(const ComponentAccess& first_access : first_accesses) {
            for(const ComponentAccess& second_access : second_accesses) {
                if(first_access.component != second_access.component)
                    continue;

                const bool first_writes =
                    first_access.mode == AccessMode::Write;

                const bool second_writes =
                    second_access.mode == AccessMode::Write;

                // Read / Read
                if(!first_writes && !second_writes)
                    continue;

                // First writes, second reads
                // First -> second
                if(first_writes && !second_writes) {
                    add_inferred_dependency(second, first);
                    continue;
                }

                // First reads, second writes
                // Second -> first
                if(!first_writes && second_writes) {
                    add_inferred_dependency(first, second);
                    continue;
                }

                // Write / Write is intentionally NOT resolved here.
                // There is no intrinsically correct ordering.
            }
        }

        for(const auto& cycle : m_inferred_cycles) {
            if(!cycle.resolved) {
                //std::cout << cycle.first << " <-> " << cycle.second << std::endl;
                throw std::logic_error(
                    "Automatic scheduler dependency inference detected "
                    "an unresolved 2-system cycle; an explicit dependency "
                    "is required"
                );
            }
        }
    }

    void Scheduler::infer_dependencies() {
        std::vector<SystemId> ids;
        ids.reserve(m_systems.size());

        for(const auto& [id, node] : m_systems)
            ids.push_back(id);

        for(size_t i = 0; i < ids.size(); i++) {
            for(size_t j = i + 1; j < ids.size(); j++) {
                infer_dependency(ids[i], ids[j]);
            }
        }
    }

    void Scheduler::validate_write_conflicts() const {
        std::vector<SystemId> ids;
        ids.reserve(m_systems.size());

        for(const auto& [id, node] : m_systems)
            ids.push_back(id);

        for(size_t i = 0; i < ids.size(); i++) {
            const auto& first = m_systems.at(ids[i]);
            for(size_t j = i + 1; j < ids.size(); j++) {
                const auto& second = m_systems.at(ids[j]);
                for(const ComponentAccess& first_access : first.accesses) {
                    if(first_access.mode != AccessMode::Write)
                        continue;

                    for(const ComponentAccess& second_access : second.accesses) {
                        if(second_access.mode != AccessMode::Write)
                            continue;

                        if(first_access.component != second_access.component)
                            continue;

                        const bool ordered =
                            has_dependency_path(ids[i], ids[j]) ||
                            has_dependency_path(ids[j], ids[i]);

                        if(!ordered) {
                            throw std::logic_error(
                                "Unresolved write/write component access conflict"
                            );
                        }
                    }
                }
            }
        }
    }

    void Scheduler::resolve_dependencies() {
        infer_dependencies();
        validate_write_conflicts();
    }

    bool Scheduler::remove_dependency(
        SystemId child,
        SystemId parent
    ) {
        auto child_it = m_systems.find(child);
        auto parent_it = m_systems.find(parent);
    
        if(child_it == m_systems.end() || parent_it == m_systems.end()) {
            return false;
        }

        auto& dependencies = child_it->second.dependencies;

        auto dependency_it = std::find_if(
            dependencies.begin(),
            dependencies.end(),
            [parent](const SystemDependency& dependency) {
                return dependency.parent == parent;
            }
        );

        if(dependency_it == dependencies.end())
            return false;

        dependencies.erase(dependency_it);

        auto& dependents = parent_it->second.dependents;

        auto dependent_it = std::find(
            dependents.begin(),
            dependents.end(),
            child
        );

        if(dependent_it != dependents.end())
            dependents.erase(dependent_it);

        return true;
    }

    void Scheduler::update(double delta, WorldContext ctx) {
        if(!m_dependencies_resolved) {
            resolve_dependencies();
            m_dependencies_resolved = true;
        }

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

                if(execute_system(id, ctx))
                    progress = true;
            }
        }
    }

    bool Scheduler::execute_system(SystemId id, WorldContext& ctx) {
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
            const double system_delta = node.schedule.interval();

            node.executor->execute(
                ctx.queue,
                ctx.clock,
                ctx.random,
                ctx.events,
                ctx.executor,
                system_delta
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