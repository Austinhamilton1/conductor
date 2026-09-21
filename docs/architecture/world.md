# World Architecture

`World` is the central simulation context in Conductor. It owns the state and runtime services required to construct and execute a simulation.

The World is intentionally a relatively thin facade: it coordinates the major kernel subsystems without exposing their internal implementation or requiring systems and plugins to manage those subsystems directly.

## Responsibilities

A `World` owns the core state of a simulation:

```text
World
+-- EntityManager
+-- ComponentRegistry
+-- ComponentStorageRegistry
+-- SimulationClock
+-- CommandQueue
+-- Scheduler
```

These subsystems have distinct responsibilities:

| Subsystem                  | Responsibility                                               |
| -------------------------- | ------------------------------------------------------------ |
| `EntityManager`            | Creates, destroys, and tracks entity lifetime                |
| `ComponentRegistry`        | Assigns and tracks component type metadata                   |
| `ComponentStorageRegistry` | Owns component storage instances                             |
| `SimulationClock`          | Tracks simulation time and timestep                          |
| `CommandQueue`             | Defers structural changes until a safe synchronization point |
| `Scheduler`                | Registers and executes simulation systems                    |

The World owns these objects so that a simulation has a single authoritative runtime context.

## World as the Simulation Boundary

Plugins interact with the simulation through the World rather than directly managing kernel subsystems.

For example, a plugin can register components and systems:

```cpp
void initialize(conductor::World& world) override {
    world.register_component<Position>();
    world.register_component<Velocity>();

    world.add_system<MovementSystem>(
        "movement",
        60.0,
        MovementSystem{}
    );
}
```

The plugin does not need to construct an `EntityManager`, `Scheduler`, or `CommandQueue`. Those services already belong to the World.

This creates a clear ownership boundary:

```text
Application
    │
  World
    │
    +-- Entity management
    +-- Component storage
    +-- System scheduling
    +-- Simulation time
    +-- Deferred commands
         │
     Plugins / Systems
```

Plugins extend the simulation, but they do not own the simulation kernel.

## Configuration and Execution

A World has two conceptual phases.

### 1. Configuration

During configuration, the simulation structure is assembled.

Typical operations include:

* registering component types
* creating entities
* adding initial component data
* registering systems
* configuring system dependencies
* configuring system frequencies
* loading and initializing plugins

During this phase, the World is mutable.

```text
World construction
       │
Plugin initialization
       │
Component registration
       │
System registration
       │
Entity/scenario setup
       │
World::finalize()
```

### 2. Simulation

After finalization, the World enters its execution phase.

The simulation repeatedly advances the World:

```cpp
world.update(delta_seconds);
```

During an update, the scheduler executes the registered systems. Systems operate on component data through their queries and may enqueue structural changes through the command queue.

The World itself remains responsible for coordinating these operations.

## Finalization

`World::finalize()` marks the boundary between configuration and simulation.

Once finalized, operations that would change the simulation's structural configuration are rejected.

Examples include:

* registering new component types
* adding new systems
* changing the simulation's structural setup

The purpose of finalization is to establish a stable runtime configuration.

```text
                 CONFIGURATION
                       │
                 World::finalize()
                       │
                  SIMULATION
```

This boundary provides several benefits:

1. **Predictable execution**
   The scheduler knows the complete set of systems that may execute.

2. **Stable component structure**
   Component types and their storage are established before simulation begins.

3. **Clear plugin lifecycle**
   Plugins configure the simulation before execution and remain loaded while their systems are active.

4. **Future optimization opportunities**
   A stable runtime structure makes later parallel execution, scheduling optimization, and GPU backends easier to implement.

Finalization is therefore not merely a convenience check. It is a fundamental lifecycle boundary in the kernel.

## Systems and World Ownership

Systems do not own the World and should not require direct access to it.

Instead, systems receive a `SystemContext` containing the runtime services they need, including their bound query, command queue, and simulation clock.

Conceptually:

```text
World
 │
 +-- Scheduler
 │      │
 │      +--- System
 │             │
 │             +-- SystemContext
 │                    +-- Query
 │                    +-- CommandQueue
 │                    +-- SimulationClock
 │
 +-- Component Storage
```

This keeps systems focused on simulation behavior rather than kernel management.

For example, a movement system should operate on position and velocity data. It should not need to know how the World stores entities or how the scheduler maintains its dependency graph.

## Deferred Structural Changes

Systems execute against the current simulation state. Structural changes are therefore deferred through the World-owned `CommandQueue`.

A system may request an operation such as:

```cpp
ctx.commands().add_component(entity, component);
```

The command is queued rather than immediately modifying the underlying storage.

At the appropriate synchronization point, the World applies the queued structural changes.

Conceptually:

```text
System execution
      │
Queue structural command
      │
Scheduler completes
      │
World applies commands
      │
Next simulation state
```

This separation prevents systems from invalidating the data structures currently being traversed and provides a foundation for future parallel system execution.

## World Updates

A World update represents one simulation step.

Conceptually:

```text
world.update(dt)
      │
Scheduler
      │
      +-- System A
      +-- System B
      +-- System C
      +-- ...
      │
Apply deferred commands
      │
Next simulation state
```

The exact execution order of systems is determined by the Scheduler and its dependency rules rather than by the order in which systems were registered.

The World therefore acts as the coordinator while the Scheduler determines system execution.

## Ownership and Lifetime

A World is non-copyable because it represents the unique runtime state of a simulation.

Its owned subsystems share the same lifetime as the World:

```text
World created
    │
    +-- Entity state created
    +-- Component storage created
    +-- Scheduler created
    +-- Command queue created
    +-- Simulation clock created
          │
      Simulation
          │
    World destroyed
    +-- Owned state destroyed
```

External objects may reference or interact with the World during its lifetime, but they do not own its core subsystems.

## Design Principle

The World exists to answer one fundamental question:

> **What constitutes the runtime state of a Conductor simulation?**

The answer is the collection of entities, components, systems, time, and deferred operations owned by the World.

The World deliberately does **not** define what the simulation means.

A drone, battery, sensor, radio, planner, or navigation algorithm is not part of the kernel. Those concepts belong in plugins and simulation-specific code.

This distinction keeps the kernel generic:

```text
                 CONDUCTOR KERNEL
 +-----------------------------------------+
 │ World                                   │
 │ +-- Entities                            │
 │ +-- Components                          │
 │ +-- Storage                             │
 │ +-- Scheduler                           │
 │ +-- Commands                            │
 │ +-- Simulation Time                     │
 +-----------------------------------------+
                       |
                       │ extension
                       │
 +-----------------------------------------+
 │ Simulation Plugins                      │
 │ +-- Drones                              │
 │ +-- Sensors                             │
 │ +-- Navigation                          │
 │ +-- Communication                       │
 │ +-- Environment                         │
 │ +-- Experiment-specific behavior        │
 +-----------------------------------------+
```

The kernel provides the machinery for simulation. Plugins provide the simulation itself.
