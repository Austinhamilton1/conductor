# Conductor

**Distributed Coordination Simulator for Heterogeneous Robotics**

Conductor is an open-source simulation and coordination platform for developing, testing, and experimenting with algorithms for **heterogeneous multi-robot systems**.

The goal is simple:

> **Make it easier to build, test, and understand systems in which many different robots must coordinate with one another.**

Conductor provides the simulation infrastructure, entity/component model, scheduling, command system, event system, and plugin architecture needed to build these systems without requiring every researcher or developer to build an entire simulation framework from scratch.

Conductor is currently in **v0.1**.

---

## Why Conductor?

Modern robotics systems increasingly involve groups of heterogeneous agents rather than a single robot operating in isolation.

A realistic system might contain:

* Aerial vehicles
* Ground vehicles
* Fixed sensors
* Mobile sensors
* Communication relays
* Planning agents
* Infrastructure
* Human-controlled systems

These agents may have different capabilities, sensors, communication constraints, update rates, and computational requirements while still needing to operate as a coordinated system.

Experimenting with these systems can be difficult.

Researchers and developers often have to build or integrate large amounts of infrastructure before they can even begin testing the algorithm they actually care about.

Conductor is intended to provide that infrastructure.

---

# What Conductor Is

Conductor is a **simulation and coordination framework** designed around modularity and experimentation.

At its core, Conductor provides a high-performance simulation environment with:

* An Entity Component System (ECS)
* Deterministic simulation clocks
* A system scheduler
* Query-based data access
* Deferred structural commands
* Event infrastructure
* Runtime plugins
* Configurable system execution frequencies
* Dependency-aware system scheduling
* Extensible simulation components
* A foundation for heterogeneous agent simulation

The architecture is designed so that the simulation engine provides the infrastructure while users provide the domain-specific behavior.

For example, Conductor itself does not need to know what a drone is.

A plugin can define:

```text
Drone
+-- Position
+-- Velocity
+-- Battery
+-- Sensor
+-- Communication
```

and then implement systems that operate on those components.

Another plugin could define a ground vehicle with an entirely different set of components.

The simulation engine remains the same.

---

# What Conductor Is Not

Conductor is intentionally **not** trying to be everything in robotics simulation.

### It is not a robotics middleware replacement

Conductor is not intended to replace systems such as ROS 2 or other robotics middleware.

Middleware focuses on communication and integration between real or simulated robotic systems.

Conductor focuses on **simulation, coordination, and experimentation**.

It may eventually integrate with external robotics ecosystems, but it does not need to replace them.

### It is not a physics engine

Conductor does not attempt to become a general-purpose physics simulator.

Physics can be provided by components, plugins, or external systems when needed.

The focus is the **coordination layer**, not reproducing every physical interaction in the world.

### It is not a finished robotics simulator

Conductor v0.1 is infrastructure.

It does not currently provide a complete collection of realistic robots, sensors, environments, planners, communication models, or mission scenarios.

Those are things that can be built on top of the platform.

### It is not a single swarm algorithm

Conductor does not prescribe one particular approach to coordination.

Different researchers should be able to experiment with:

* Centralized planning
* Distributed planning
* Consensus algorithms
* Task allocation
* Formation control
* Multi-agent reinforcement learning
* Communication-aware coordination
* Heterogeneous planning
* Custom experimental algorithms

The platform should provide the infrastructure without dictating the research.

---

# v0.1 — The Foundation

Conductor v0.1 establishes the core simulation architecture.

The emphasis of this release is **infrastructure rather than visual polish**.

The engine currently provides the foundational pieces required to construct simulations programmatically.

## Entity Component System

Conductor uses an ECS architecture for representing simulation state.

Components are data-oriented and systems operate on the component sets they require through queries.

Systems can explicitly declare read and write access to component types.

This allows the scheduler to reason about dependencies between systems.

See:

* [`docs/ecs.md`](docs/ecs.md)

## System Scheduler

Systems are scheduled according to their declared component access patterns and explicit dependencies.

Conductor can infer dependencies from query access patterns, allowing systems to express their requirements through their data access rather than manually specifying every relationship.

Systems can also have explicit dependencies when domain-specific ordering is required.

The scheduler detects invalid dependency graphs, including cycles.

See:

* [`docs/scheduler.md`](docs/scheduler.md)

## Deferred Commands

Systems do not directly perform structural changes to the ECS while executing.

Instead, structural operations are submitted as commands and applied at a defined synchronization point.

This provides predictable structural mutation semantics and establishes a foundation for future parallel and heterogeneous execution.

See:

* [`docs/commands.md`](docs/commands.md)

## Events

Conductor provides an event system for communicating transient occurrences between systems without requiring those systems to be directly coupled.

## Simulation and Application Lifecycle

The engine provides explicit application and simulation lifecycle management, including initialization, update, and shutdown phases.

See:

* [`docs/lifecycle.md`](docs/lifecycle.md)

## Plugins

Simulation functionality can be packaged into runtime plugins.

Plugins can:

* Register component types
* Register systems
* Define simulation behavior
* Participate in application initialization and shutdown

This is an important part of Conductor's long-term goal of allowing researchers and developers to extend the simulator without modifying the engine itself.

## Testing

The v0.1 foundation is covered by extensive unit and integration testing across the core engine systems.

The project uses:

* C++23
* CMake
* Catch2
* CTest

---

# v0.1 Philosophy

Conductor v0.1 intentionally prioritizes:

**Correctness -> Clear architecture -> Extensibility -> Performance**

rather than attempting to optimize every subsystem prematurely.

The first version of the engine establishes a clean foundation that can be benchmarked and optimized as real simulation workloads emerge.

This also keeps the architecture flexible enough to support future execution backends.

---

# What Conductor Will Become

The long-term goal for Conductor is substantially larger than the current v0.1 engine.

By **v1.0**, Conductor is intended to be a practical platform for researchers and developers to build and evaluate heterogeneous multi-agent coordination systems.

The exact feature set will evolve based on real usage, but the direction is clear.

## A Complete Multi-Agent Simulation Environment

Users should be able to construct simulations containing heterogeneous agents with different:

* Capabilities
* Sensors
* Actuators
* Communication models
* Planning systems
* Control systems
* Update frequencies
* Computational requirements

The simulation engine should remain independent of the particular robotic platforms being simulated.

---

## A Plugin-Based Research Platform

A major goal of Conductor is to make experimentation modular.

A researcher should be able to implement a new:

* Planner
* Sensor model
* Communication model
* Coordination algorithm
* Controller
* Agent type
* Optimization strategy

without rebuilding the simulation engine itself.

The plugin architecture established in v0.1 is the beginning of this model.

---

## High-Performance Simulation

Conductor is intended to support simulations containing large numbers of agents while maintaining predictable performance.

The initial backend is CPU-based.

As the architecture matures, performance-critical execution can be moved to more specialized hardware where appropriate.

One long-term direction is a GPU execution backend based on Vulkan compute.

The goal is not simply to make individual operations fast, but to make **large-scale multi-agent simulation practical**.

---

## Coordination and Communication Modeling

Future versions will increasingly focus on the problems that make distributed robotics difficult.

These include:

* Limited communication
* Communication latency
* Packet loss
* Network topology
* Distributed state
* Partial observability
* Agent failures
* Dynamic environments
* Resource constraints

This will allow algorithms to be evaluated under conditions that more closely resemble real distributed robotic systems.

---

## Planning and Decision Making

Conductor will provide infrastructure for experimenting with different approaches to multi-agent decision making.

The platform should make it possible to compare different strategies within the same simulation environment rather than requiring researchers to build separate experimental infrastructure for every algorithm.

---

# The Long-Term Vision

The ultimate vision for Conductor is a platform where the **coordination problem becomes the interesting part**, rather than the simulation infrastructure.

A researcher should be able to start with a question such as:

> *How should 100 heterogeneous robots coordinate when communication is limited and some agents fail?*

and spend their time implementing and evaluating the answer rather than building:

* An ECS
* A scheduler
* An event system
* A plugin loader
* A simulation clock
* A command system
* A visualization framework
* A custom execution engine

Conductor is intended to provide that foundation.

---

# Roadmap

The roadmap is intentionally incremental.

### v0.1 — Core Engine

**Status: Complete**

* [x] Entity Component System
* [x] Component registration
* [x] Entity lifecycle
* [x] Queries
* [x] Read/write access tracking
* [x] System scheduler
* [x] Dependency inference
* [x] Explicit dependencies
* [x] Cycle detection
* [x] Deferred structural commands
* [x] Events
* [x] Simulation clocks
* [x] Application lifecycle
* [x] Runtime plugins
* [x] System frequency control
* [x] Unit and integration testing
* [x] Core architecture documentation

### Future Development

The next stages will focus increasingly on the user-facing simulation experience.

Potential areas include:

* [ ] Rich agent and sensor models
* [ ] Communication/network simulation
* [ ] Planning and coordination modules
* [ ] Visualization
* [ ] Example multi-agent scenarios
* [ ] Simulation configuration
* [ ] Performance benchmarking
* [ ] Larger-scale simulations
* [ ] GPU execution backend
* [ ] Research-oriented tooling
* [ ] Enterprise capabilities

The roadmap will evolve as Conductor is used to build real simulations.

---

# Architecture

Conductor is composed of several core subsystems.

```text
                         +----------------------+
                         │      Application     │
                         +----------------------+
                                    │
              +-------------------------------------------+
              │                     │                     │
        +-----------+         +-----------+        +------------+
        │   World   │         │  Plugins  │        │   Events   │
        +-----------+         +-----------+        +------------+
              │
       +--------------+
       │              │
    +-------+    +-----------+
    │  ECS  │    │ Scheduler │
    +-------+    +-----------+
        │              │
        │         +----------+
        │         │ Systems  │
        │         +----------+
        │              │
    Components      Queries
        │              │
        +--------------+
               |
               |
          Simulation State
```

For detailed architectural documentation, see:

* [`docs/overview.md`](docs/overview.md)
* [`docs/ecs.md`](docs/ecs.md)
* [`docs/world.md`](docs/world.md)
* [`docs/scheduler.md`](docs/scheduler.md)
* [`docs/lifecycle.md`](docs/lifecycle.md)
* [`docs/commands.md`](docs/commands.md)

---

# Building

Conductor currently requires:

* C++23 compiler
* CMake 3.25+
* Catch2 3.x

Clone the repository and configure the project with CMake:

```bash
git clone --recurse-subdirectories https://github.com/Austinhamilton1/conductor.git
cd conductor

cmake --preset debug
cmake --build --preset debug
```

Run the test suite with:

```bash
ctest --preset debug
```

See the project documentation for additional build and development information.

---

# Project Status

Conductor is **early-stage software**.

v0.1 represents the completion of the initial engine foundation, not a finished end-user simulation product.

APIs and architecture may continue to change as the project moves toward v1.0.

The project is currently focused on establishing a strong technical foundation and discovering which simulation and coordination capabilities provide the most value to users.

---

# Contributing

Conductor is intended to become an open platform for experimentation and research.

As the project matures, contributions in areas such as:

* Simulation models
* Coordination algorithms
* Sensors
* Planners
* Communication models
* Visualization
* Performance
* Documentation
* Testing

will be valuable.

More contribution guidelines will be added as the project approaches a public v1.0 ecosystem.

---

# License

Conductor is provided under an Apache 2.0 license.

---

## The Goal

Conductor exists to make distributed robotics experimentation more accessible.

The long-term objective is to provide a platform where building a sophisticated multi-agent simulation is no longer primarily a software-infrastructure problem.

Instead, the infrastructure should disappear into the background.

**The robots, the environment, the communication network, and the coordination algorithms should be the interesting part.**

---

## v0.2 Vision

Version 0.2 of Conductor should build upon v0.1. The release of v0.1 brought with it the initial simulation engine; however this engine is only a small part of the Conductor Simulator vision. Version 0.2 will streamline some features of v0.1 and begin to forge the user-facing API, making Conductor a better product for users.

Some goals of v0.2 include:

 - [X] Application should own a `Random` instance, there should no longer be a need for an external random number generator.
 - [X] Deterministic `SimulationClock` execution. `SimulationClock` should increment deterministically regardless of time scale.
 - [X] `Application` should become configurable. There should be no reason to inherit a new `Application` class for every application instance.
 - [X] Configurable entity creation. A user should be able to configure 1, 100, or 10,000 drones without having to create them with an explicit loop.

---

## v0.3 Vision

Version 0.3 of Conductor should create the first standalone Conductor Simulator. The main goal of this version of the simulator is to create a single application that users can link against without recompilation. This version is the first step of converting Conductor from a generic C++ framework to an actual product.

Some goals of v0.3 include:

 - [X] Compilation of a single, generic `conductor` application that can run multiple simulation configurations. 
 - [X] Plugins become first-class objects of the simulation. Users should be able to create plugins instead of recompiling the conductor application.
 - [X] YAML configuration for runtime `Application`.
 - [X] YAML configuration for entity spawning.
 - [X] YAML configuration for plugin loading.
 - [X] Relative paths from YAML configuration.
 - [X] `SimulationClock` exposed through `SystemContext` to give systems access to global time.

---

## v0.4 Vision

Version 0.4 of Conductor should freeze v1.0 of the plugin API for Conductor. After v0.4, researchers should be able to easily create plugins with a stable, declarative C++ API.

Some goals of v0.4 include:

 - [X] Rename `Plugin::initialize()` to `Plugin::configure()`.
 - [X] Remove `World` from the plugin lifecycle API.
 - [X] Add `PluginConfig` as the plugin configuration API.
 - [X] Add `PluginConfig::component<T>()`.
 - [X] Add `PluginConfig::system<T>()`.
 - [X] Return `SystemHandle` from `system`.
 - [X] Add entity spawning from `PluginConfig`,
 - [X] Expose evnets through `PluginConfig`.
 - [X] Expose event publishing through `SystemContext`.

## v0.4 Benchmarks

Version 0.4 has a plugin that can be run. The plugin spawns a set number of drones and moving targets. At each timestep, the drones must follow the targets. As the drones move, their battery decreases at a rate that is porportional to their speed. It is a simple simulation meant to show the API layout for Conductor v0.4.

This simulation was benchmarked on two computers. The first computer is a desktop Ubuntu 24.04 machine with an Intel Core i7-8700 12 core CPU with 32 GB RAM. The second computer is a laptop Pop!_OS 24.04 machine with an Intel Core Ultra 9 275HX and 16 GB RAM.

On both machines, the simulation was run for 10 simulation seconds with 10, 100, and 1000 drones. It was run at 1x, 10x, 100x, and 1000x speed.

### i7 Benchmark Results:
| Num Drones | Simulation Speed | Simulation Time | Real Time | Observed Simulation Speed |
| ---------- | ---------------- | --------------- | --------- | ------------------------- |
| 10 | 1.0x | 10.0s | 10.005s | ~1.0x |
| 10 | 10.0x | 10.0s | 1.003s | 9.97x |
| 10 | 100.0x | 10.0s | 0.103s | 97.09x |
| 10 | 1000.0x | 10.0s | 0.014s | 714.29x |
| 100 | 1.0x | 10.0s | 10.003s | ~1.0x |
| 100 | 10.0x | 10.0s | 1.003s | 9.97x |
| 100 | 100.0x | 10.0s | 0.103s | 97.09x |
| 100 | 1000.0x | 10.0s | 0.012s | 833.33x |
| 1000 | 1.0x | 10.0s | 10.005s | ~1.0x |
| 1000 | 10.0x | 10.0s | 1.003s | 9.97x |
| 1000 | 100.0x | 10.0s | 0.103s | 97.09x |
| 1000 | 1000.0x | 10.0s | 0.053s | 188.68x |

### i9 Benchmark Results:
| Num Drones | Simulation Speed | Simulation Time | Real Time | Observed Simulation Speed |
| ---------- | ---------------- | --------------- | --------- | ------------------------- |
| 10 | 1.0x | 10.0s | 10.004s | ~1.0x |
| 10 | 10.0x | 10.0s | 1.004s | 9.96x |
| 10 | 100.0x | 10.0s | 0.104 | 96.15x |
| 10 | 1000.0x | 10.0s | 0.014 | 714.29x |
| 100 | 1.0x | 10.0s | 10.005s | ~1.0x |
| 100 | 10.0x | 10.0s | 1.004s | 9.96x |
| 100 | 100.0x | 10.0s | 0.104s | 96.15x |
| 100 | 1000.0x | 10.0s | 0.014s | 714.29x |
| 1000 | 1.0x | 10.0s | 10.005s | ~1.0x |
| 1000 | 10.0x | 10.0s | 1.004s | 9.96x |
| 1000 | 100.0x | 10.0s | 0.104s | 96.15x |
| 1000 | 1000.0x | 10.0s | 0.037s | 270.27x |


### Benchmark Observations

Two things should be noted about the benchmarks above. Namely:

 - Conductor runtime grows linearly with respect to drone count. This means that when optimizing in future versions, the algorithmic complexity of Conductor is not the main limiting factor.
 - Increasing simulation speed increases simulation runtime, but after a certain point, Conductor seems to hit a "speed limit". For example, with 1000 drones in the above simulation, increasing the simulation time-scale does not affect the runtime for `time_scale > 100.0`. This means that systems are scheduled and ran efficiently, but systems are taking time to finish.

These both point to a natural conclusion that the main bottleneck of Conductor is not the simulation infrastructure/design, but rather the speed at which systems are able to complete. This means that for performance optimizations, approaches such as multithreading, SIMD multiprocessing, and component retrieval will most likely succeed if done correctly.

---

## v0.5 Vision

Version 0.5 of Conductor will introduce parallelism to several subsystems within the Conductor engine. The scheduler, command queue, and query subsystems were all designed with parallelism in mind. v0.5 should optimize the runtime of large-scale simulations by running independent subsystems concurrently.

Some goals of v0.5 include:

 - [ ] Parallelize scheduler. Scheduler should be able to run independent systems concurrently, reducing the overall time to run systems.
 - [X] Parallelize command queue. Commands that are queued for execution during a single frame should be able to run concurrently. Commands are atomic, so running several in parallel should not affect the execution of the command queue.
 - [X] Parallelize queries. Sufficiently large queries should be able to run with chunked workers.
 - [X] Change query API. Queries should be able to run either single threaded or multithreaded. Proposed API change is to add `.do(function())` to query, which runs parallel workers.

## v0.5 Considerations

There are several dependencies within Conductor. Parallel execution of different subsystems will inevitably affect other subsystems that need to be made thread safe. Those subsystems include:

### `Scheduler`

 - [ ] `Random`: The random number generator needs to be made thread safe. Making this multithreaded will break determinism, therefore users need to be able to define whether they want multithreading or not.
 - [X] `SimulationClock`: This should be made constant when passing through dependency injection to ensure no changes are made that race.
 - [X] `CommandQueue`: There is no guarantee that two threads will not race on this. This needs to be made thread safe. Start with mutex, then move on to MPMC queue.
 - [X] `EventBus`: This also needs to be made thread safe. Start with mutex, then move on to MPMC queue.

### `CommandQueue`

 - [X] Internal representation: `CommandQueue` uses three internal vectors. These need to be made thread safe so they do not race.
 - [X] `EntityManager`: This will need to be made thread safe. `CommandQueue` uses this to create/destroy entities. Entities may be created/destroyed in parallel.
 - [X] `ComponentStorage/ComponentStorageRegistry`: This will also need to be made thread safe. This is used to create/destroy components. Components may be created/destroyed in parallel.

### `Query`

- [X] Making `Query` parallel should not affect any other subsystems. `Query` workers should be independent. Each query contains one instance of every entity, so there should be no write-write conflicts.