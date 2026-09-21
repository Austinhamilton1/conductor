# Scheduler Architecture

The Scheduler is responsible for managing and executing simulation systems.

It determines **when systems execute**, **which systems may execute relative to one another**, and **which dependencies must be satisfied before a system can run**.

The Scheduler does not own the simulation state itself. Instead, it operates on systems that access state through ECS queries and execute within the runtime context provided by the World.

## Responsibilities

The Scheduler is responsible for:

* registering systems
* assigning runtime system IDs
* tracking system lifetime
* managing system dependencies
* detecting dependency conflicts and cycles
* inferring dependencies from system data access
* controlling execution frequency
* pausing, resuming, and cancelling systems
* executing systems when their scheduling requirements are satisfied

The Scheduler is owned by the World.

```text
World
 │
 +-- Scheduler
      │
      +-- System A
      +-- System B
      +-- System C
      +-- ...
```

The Scheduler therefore manages execution, but does not define what a system actually does.

## Systems

A system represents a unit of simulation behavior.

A system declares the component data it requires through its query.

For example:

```cpp
class MovementSystem {
public:
    using Query = conductor::Query<
        conductor::Read<Velocity>,
        conductor::Write<Position>
    >;

    template<typename BoundQuery>
    void update(conductor::SystemContext<BoundQuery>& ctx) {
        for (auto [velocity, position] : ctx.query()) {
            // Update position from velocity.
        }
    }
};
```

The Scheduler does not need to understand the implementation of `MovementSystem`.

It only needs to know:

* the system's identity
* its query/data access
* its execution frequency
* its dependencies
* its current scheduling state

This separation allows systems to contain simulation-specific behavior without coupling them to the Scheduler's implementation.

## System Registration

Systems are registered through the World:

```cpp
world.add_system<MovementSystem>(
    "movement",
    60.0,
    MovementSystem{}
);
```

The World binds the system's query to the appropriate component storage and then registers the resulting system with the Scheduler.

Conceptually:

```text
World::add_system()
       │
       +-- Bind Query
       │
    Scheduler
       │
       +-- Assign system ID
       +-- Register system
       +-- Analyze dependencies
       +-- Store scheduling state
```

The application and plugins interact with this process through `World`.

The Scheduler itself is an internal runtime service rather than something that simulation plugins need to manage directly.

## System Identity

Every registered system receives a runtime identifier.

This identifier distinguishes individual system instances, even when multiple instances use the same C++ system type.

For example, two instances of the same system type may represent different simulation behaviors:

```text
System Type: NavigationSystem

Instance A → System ID 1
Instance B → System ID 2
```

The Scheduler therefore treats system identity as a property of the registration instance rather than simply the C++ type.

This allows multiple instances of the same system type to coexist.

## System Handles

Registering a system returns a `Scheduler::SystemHandle`.

The handle represents the lifetime and control interface for the registered system.

A handle can be used to perform operations such as:

* cancel a system
* pause a system
* resume a system
* change its execution frequency
* establish dependencies

Conceptually:

```text
add_system()
     │
System registered
     │
SystemHandle
     │
     +-- pause()
     +-- resume()
     +-- cancel()
     +-- set_frequency()
     +-- depends_on()
```

### Handle Lifetime

The handle is also part of the system's lifetime management.

When the handle is destroyed, its associated system is removed from the Scheduler.

Therefore, a system that is intended to remain active must have a handle whose lifetime covers the desired lifetime of that system.

For a plugin-owned system, the plugin can retain the handle as member state:

```cpp
class MyPlugin : public conductor::Plugin {
private:
    conductor::Scheduler::SystemHandle movement_handle;
};
```

Then:

```cpp
movement_handle =
    world.add_system<MovementSystem>(
        "movement",
        60.0,
        MovementSystem{}
    );
```

This keeps the system registered for the lifetime of the plugin.

The following pattern is therefore different:

```cpp
world.add_system<MovementSystem>(
    "movement",
    60.0,
    MovementSystem{}
);
```

If the returned handle is immediately destroyed, the associated system does not remain registered.

This lifetime relationship is intentional: the handle acts as the owner's control and lifetime token for the registered system.

## Execution Frequency

Systems may specify an expected execution frequency.

For example:

```cpp
world.add_system<NavigationSystem>(
    "navigation",
    10.0,
    NavigationSystem{}
);
```

A frequency of `10.0` represents an expected execution interval of approximately:

```text
1 / 10 = 0.1 seconds
```

This allows different systems to operate at different simulation rates.

For example:

```text
60 Hz   → Movement
30 Hz   → Sensor processing
10 Hz   → Navigation
 1 Hz   → Battery reporting
```

The Scheduler uses the system's configured frequency to determine when that system is eligible to execute.

Frequency is therefore a scheduling property rather than a property of the system's implementation.

## Pausing and Resuming

A system can be temporarily paused through its handle.

A paused system remains registered but is not executed until it is resumed.

```text
Active
  │
  +-- pause()
  |
Paused
  │
  +-- resume()
  |
Active
```

This differs from cancellation.

Pausing preserves the system's registration, while cancellation removes it from active scheduling.

## Cancellation

A system can be cancelled through its handle.

Cancellation removes the system from the Scheduler.

Conceptually:

```text
Registered
    │
    +-- cancel()
    |
Removed
```

A cancelled system is no longer part of the active execution graph.

## Dependencies

Systems may depend on other systems.

A dependency expresses an execution requirement:

> System B must not execute until System A has completed its required execution.

For example:

```text
Navigation
    │
Movement
```

This means Movement depends on Navigation.

The Scheduler represents dependencies explicitly rather than relying on registration order.

### Why Dependencies Exist

Simulation behavior often requires a specific ordering.

For example:

```text
Sensor Update
      │
Navigation
      │
Movement
```

A sensor system may produce state required by navigation, which in turn produces state consumed by movement.

The dependency graph expresses this relationship directly.

Without dependencies, registration order should not be assumed to establish execution order.

## Dependency Graph

The Scheduler maintains a directed dependency graph.

Conceptually:

```text
        Sensor
          │
      Navigation
          │
       Movement
```

Each edge represents an execution constraint.

If:

```text
A -> B
```

then B depends on A.

The Scheduler uses this graph to determine which systems can execute and which systems must wait.

## Dependency Validation

The Scheduler validates dependency relationships when they are created.

Invalid relationships include:

### Invalid child

The system being made dependent does not exist.

```text
Unknown → Existing
```

### Invalid parent

The dependency target does not exist.

```text
Existing → Unknown
```

### Self dependency

A system cannot depend on itself.

```text
A → A
```

### Duplicate dependency

The same dependency cannot be registered more than once.

```text
A → B
A → B
```

### Cycle

Dependencies must form a directed acyclic graph.

For example:

```text
A → B
B → C
C → A
```

would create a cycle.

A cycle prevents the Scheduler from establishing a valid execution order and is therefore rejected.

## Dependency Results

Dependency operations report the reason an operation failed rather than treating every failure as the same error.

The Scheduler can distinguish conditions such as:

```text
Success
InvalidChild
InvalidParent
SelfDependency
DuplicateDependency
CycleDetected
```

This allows callers and tests to distinguish invalid configuration from a valid dependency operation.

## Automatic Dependency Inference

Conductor can infer dependencies from system data access.

Because systems declare their ECS queries using `Read<T>` and `Write<T>`, the Scheduler can reason about potential data conflicts.

For example:

```text
System A
Read<Position>

System B
Write<Position>
```

The systems access the same component, and one writes to it.

The Scheduler can therefore identify that their execution may require coordination.

This creates the following architectural relationship:

```text
             ECS
              │
       Query access metadata
              │
          Scheduler
              │
       conflict analysis
              │
        Dependencies
```

## Read/Write Relationships

The Scheduler considers the access mode of each system.

### Read / Read

```text
A: Read<Position>
B: Read<Position>
```

Both systems only read the component.

There is no write conflict between them.

### Read / Write

```text
A: Read<Position>
B: Write<Position>
```

The systems access the same component and one modifies it.

Execution may therefore require ordering.

### Write / Read

```text
A: Write<Position>
B: Read<Position>
```

This is the same conflict relationship in the opposite declaration order.

### Write / Write

```text
A: Write<Position>
B: Write<Position>
```

Both systems modify the same component.

This is a direct write/write conflict.

An ordering dependency is required unless the runtime can otherwise establish that the systems are independent.

## Explicit vs. Inferred Dependencies

There are two sources of scheduling relationships.

### Inferred dependencies

The Scheduler derives potential conflicts from system queries.

```text
Query
  │
Read/Write analysis
  │
Inferred dependency
```

### Explicit dependencies

The application or plugin directly specifies an ordering requirement.

```text
SystemHandle
      │
depends_on(...)
```

Explicit dependencies are useful when ordering is required for reasons that are not represented by component access.

For example, two systems might operate on independent data but still need a specific order because one establishes an application-level state transition consumed by the other.

## Explicit Dependencies and Inference

The Scheduler treats explicit dependencies as authoritative configuration when the caller specifies them.

Automatic inference provides the default dependency structure derived from data access, while explicit dependencies allow simulation-specific ordering requirements to be expressed directly.

This provides two levels of control:

```text
Default
   │
Infer from data access
   │
Override / supplement with
explicit dependencies
```

The exact dependency configuration remains part of the simulation's structural configuration and should therefore be established before World finalization.

## Execution

During a simulation update, the World asks the Scheduler to execute its registered systems.

Conceptually:

```text
World::update(dt)
       │
   Scheduler
       │
       +-- Determine eligible systems
       │
       +-- Check dependencies
       │
       +-- Check frequency
       │
       +-- Check paused/cancelled state
       │
 Execute systems
```

A system executes only when its scheduling conditions are satisfied.

Its `SystemContext` provides the runtime resources required for execution.

```text
System
  │
SystemContext
+-- Query
+-- CommandQueue
+-- SimulationClock
```

The system does not need direct access to the Scheduler.

## Systems and Deferred Commands

A system may modify existing component values directly through its query.

Structural changes are instead submitted to the World-owned command queue.

For example:

```cpp
for (auto [velocity, position] : ctx.query()) {
    position.x += velocity.x * ctx.clock().delta_seconds();
}
```

The system can also queue structural work:

```cpp
ctx.commands().add_component(
    entity,
    Battery{100.0f}
);
```

The Scheduler therefore executes system behavior while the CommandQueue provides a controlled mechanism for structural changes.

```text
Scheduler
    │
System execution
    │
    +-- Read/write component data
    │
    +-- Queue structural changes
             │
       Command processing
```

## Scheduler and World

The Scheduler is intentionally owned by the World rather than exposed as an independent application-level subsystem.

This keeps the public simulation interface simple:

```cpp
world.add_system<MovementSystem>(...);
world.update(dt);
```

rather than requiring application code to coordinate several kernel services manually.

The World provides the boundary between simulation configuration and kernel execution.

The Scheduler remains responsible for one specific concern:

> **Given the configured systems and their relationships, determine which systems may execute and execute them accordingly.**

## Scheduler Lifecycle

The Scheduler follows the World lifecycle.

### Configuration

During configuration:

```text
Create system
     │
Bind query
     │
Register system
     │
Assign ID
     │
Establish dependencies
```

### Finalization

When the World is finalized, the configured system set becomes part of the simulation's runtime structure.

### Execution

During simulation:

```text
World::update()
      │
Scheduler
      │
Eligible systems
      │
System execution
```

### Teardown

When the World is destroyed, its Scheduler and the systems it owns are destroyed as part of World teardown.

Systems implemented by plugins must therefore be destroyed while their plugin libraries remain loaded.

## Design Goals

The Scheduler is designed around several principles.

### Explicit execution semantics

Dependencies should describe actual execution requirements rather than relying on registration order.

### Data-aware scheduling

System queries expose read/write requirements that allow the Scheduler to reason about data conflicts.

### Controlled mutation

Systems operate on stable component storage during execution, with structural changes deferred through the CommandQueue.

### Independent system instances

The Scheduler identifies systems by runtime instance rather than C++ type, allowing multiple instances of the same system type.

### Explicit lifetime

`SystemHandle` makes system lifetime and scheduler control explicit.

### Future parallelism

The dependency graph and read/write access model provide the information needed to determine which systems could eventually execute concurrently.

The current scheduler does not need to be parallel to benefit from this architecture. The dependency model establishes the information required for future execution strategies.

## Scheduler Mental Model

The Scheduler can be summarized as:

```text id="g1b5f4"
                 System
                    │
                  Query
                    │
             Read / Write data
                    │
               Dependency
                Analysis
                    │
             +-------------+
             │             │
         Inferred       Explicit
         relations      relations
             │             │
             +-------------+
                    |
             Execution Graph
                    │
               Scheduler
                    │
              SystemContext
                    │
              System Update
```

The Scheduler therefore connects the ECS data model to runtime execution.

The ECS describes **what a system accesses**.

The dependency graph describes **what must happen before what**.

The Scheduler determines **when each system executes**.

## Design Principle

The central idea behind the Conductor Scheduler is:

> **Systems declare what they need; dependencies describe what must happen first; the Scheduler turns those constraints into execution.**

The Scheduler does not define simulation behavior.

It provides the execution machinery that allows independently implemented systems to cooperate within a predictable simulation runtime.
