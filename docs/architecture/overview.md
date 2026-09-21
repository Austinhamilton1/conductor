# Conductor Engine Architecture

Conductor is a modular simulation engine designed for distributed and heterogeneous robotics. Its architecture separates the simulation kernel from the behaviors, models, and coordination algorithms being simulated.

The engine kernel provides the infrastructure required to construct and execute a simulation:

* Entity and component management
* Data-oriented component storage
* System execution
* Dependency scheduling
* Deferred structural commands
* Simulation timing
* Event dispatch
* Plugin loading and lifecycle management

Simulation-specific behavior is implemented on top of this kernel through systems and plugins.

## Architectural Model

At a high level, a Conductor simulation follows this structure:

```text
                    Application
                         |
                   Plugin Manager
                         │
                       World
              +----------+----------+
              │          │          │
          Entities    Components  Systems
              │          │          │
              +----------+----------+
                         │
                     Scheduler
                         │
                  Simulation Update
```

The `World` is the central simulation context. It owns the state and execution infrastructure required by a simulation, while systems operate on that state through queries.

Plugins extend the world during the configuration phase by registering components and systems.

## Core Design Principles

### Immutable Simulation Structure

A Conductor world has two distinct phases:

1. **Configuration**
2. **Simulation**

During configuration, plugins and the application construct the simulation:

* Components are registered.
* Component storage is created.
* Systems are registered.
* System dependencies are established.
* Initial entities and state are created.

Once the world is finalized, its structural configuration becomes immutable.

```text
Configuration
      │
 World::finalize()
      │
   Simulation
```

This boundary allows the engine to make stronger assumptions during simulation and prevents systems from modifying the structure of the simulation while it is executing.

### Data-Oriented State

Simulation state is represented primarily through entities and components rather than through large object hierarchies.

Entities are lightweight identifiers. Components contain simulation data. Systems contain behavior.

This separation allows systems to operate over contiguous component storage and keeps simulation state independent from the code that processes it.

### Explicit Execution Dependencies

Systems are scheduled according to their declared data access and dependencies.

A system describes the components it reads and writes through its query. The scheduler uses this information, along with explicit dependencies, to establish a valid execution order.

The goal is to make system execution predictable while allowing independent systems to execute without unnecessary serialization.

### Deferred Structural Changes

Systems do not directly modify the structural layout of the world while they are executing.

Operations such as:

* creating entities
* destroying entities
* adding components
* removing components

are submitted as commands and applied at a defined synchronization point.

This separates simulation logic from structural mutation and provides a foundation for future parallel execution.

### Plugin-Based Extension

Simulation-specific functionality does not belong in the engine kernel.

Plugins can register:

* Components
* Component storage
* Systems
* Simulation behavior

A plugin is loaded during configuration and remains alive for the duration of the simulation.

This allows the engine to remain generic while individual simulations define their own robotics models, sensors, communication systems, planners, and coordination algorithms.

## Engine Lifecycle

A typical Conductor simulation follows this lifecycle:

```text
Application Construction
          │
Plugin Loading
          │
World Configuration
          │
          +-- Register Components
          +-- Create Initial Entities
          +-- Register Systems
          +-- Configure Dependencies
          │
World Finalization
          │
Simulation
          │
          +-- Process Events
          +-- Execute Systems
          +-- Apply Deferred Commands
          │
Plugin Shutdown
          │
World Teardown
          │
Plugin Destruction
```

The exact execution sequence is controlled by the `Application` and `World`, but the configuration/simulation boundary is fundamental to the engine architecture.

## Kernel Responsibilities

The engine kernel is intentionally limited to infrastructure.

It is responsible for:

* Managing simulation state
* Executing systems
* Maintaining execution dependencies
* Managing simulation time
* Handling structural commands
* Dispatching events
* Managing plugin lifetimes

The kernel does **not** define a specific robotics model.

For example, the engine does not inherently know what a drone, sensor, waypoint, battery, or radio is.

Those concepts belong to simulation plugins.

This distinction is central to Conductor's architecture:

```text
             Conductor Kernel
        +-------------------------+
        │ ECS                     │
        │ Scheduler               │
        │ Commands                │
        │ Events                  │
        │ Clock                   │
        │ World                   │
        │ Plugin Management       │
        +-------------------------+
                     │
                     │ extension
                     ▼
        +-------------------------+
        │ Simulation Plugins      │
        │                         │
        │ Drone Models            │
        │ Sensors                 │
        │ Networks                │
        │ Planners                │
        │ Coordination Algorithms │
        +-------------------------+
```

Keeping this boundary clear allows the same kernel to support many different simulations without embedding domain-specific assumptions into the engine.
