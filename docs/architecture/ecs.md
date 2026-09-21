# Entity Component System

Conductor uses an Entity Component System (ECS) to represent simulation state.

The ECS separates **identity**, **data**, and **behavior**:

* **Entities** provide identity.
* **Components** provide simulation data.
* **Systems** provide behavior.

This separation allows simulation state to remain independent from the code that operates on it.

## Overview

The basic relationship is:

```text
Entity
  │
  +-- Component A
  +-- Component B
  +-- Component C
         │
      Systems
```

For example, a simulated drone might be represented as:

```text
Entity: Drone 42
+-- Position
+-- Velocity
+-- Orientation
+-- Battery
```

A movement system might operate on `Position` and `Velocity`, while a battery system operates on `Battery`.

The ECS does not need to know that these components represent a drone. They are simply typed pieces of simulation state.

## Entities

An entity represents an object that exists within the simulation.

Entities do not contain the simulation data themselves. Instead, an entity identifies which component data belongs to that object.

Conductor entities use an index and generation:

```text
Entity
+-- index
+-- generation
```

The index identifies a slot, while the generation distinguishes different lifetimes of that slot.

Conceptually:

```text
Entity { index = 42, generation = 3 }
```

If the entity is destroyed and its index is later reused:

```text
Old entity:
{ index = 42, generation = 3 }

Destroyed
      │
Index 42 becomes reusable
      │
New entity:
{ index = 42, generation = 4 }
```

The generation prevents stale references to the old entity from being treated as references to the new entity.

### Entity Lifetime

Entity lifetime is managed by the World.

Typical creation is:

```cpp
auto entity = world.create_entity();
```

Components can then be associated with the entity.

When an entity is destroyed, its associated component data is removed as part of entity cleanup.

Entities therefore represent **identity and lifetime**, not data storage.

## Components

Components represent simulation state.

A component should be a small, data-oriented type describing one aspect of an entity.

For example:

```cpp
struct Position {
    float x;
    float y;
    float z;
};

struct Velocity {
    float x;
    float y;
    float z;
};
```

The ECS intentionally keeps components independent from systems.

A component does not contain the behavior that operates on it. Instead, systems read and modify component data.

### Component Requirements

Conductor components are designed to be suitable for efficient storage and future execution backends.

Components are required to satisfy the engine's component concept, including:

* trivially copyable
* standard layout

This keeps component data simple and predictable.

It also avoids requiring the ECS to understand arbitrary object lifetime behavior for every component type.

The constraint is intentional: simulation state should be representable as straightforward data.

## Component Registration

Component types are registered with the World before simulation begins.

Conceptually:

```cpp
world.register_component<Position>();
world.register_component<Velocity>();
```

Registration gives the component type an internal `ComponentTypeId` and establishes the metadata required by the ECS.

The registry tracks information such as:

```text
Component
+-- Type ID
+-- Size
+-- Alignment
```

The type ID allows the runtime to identify component types without requiring the scheduler or storage registry to know the C++ type at every boundary.

## Component Storage

Component data is stored separately from entities.

For each registered component type, the World owns a corresponding component storage.

Conceptually:

```text
World
│
+-- Position Storage
│   +-- Position
│   +-- Position
│   +-- Position
│
+-- Velocity Storage
│   +-- Velocity
│   +-- Velocity
│   +-- Velocity
│
+-- Battery Storage
    +-- Battery
    +-- Battery
```

This is fundamentally different from storing an entire entity as one large object:

```text
Entity
+-- Position
+-- Velocity
+-- Battery
+-- Orientation
+-- ...
```

Instead, each component type has its own storage.

This allows systems that only need a subset of the simulation state to access that subset directly.

## Storage Layout

Conductor's component storage uses chunked storage.

A storage contains fixed-size chunks, with component data and the corresponding entity identifiers maintained together.

Conceptually:

```text
ComponentStorage<Position>

Chunk 0
+---------------------------------+
│ Position │ Position │ Position  │
│ Entity   │ Entity   │ Entity    │
+---------------------------------+

Chunk 1
+---------------------------------+
│ Position │ Position │ Position  │
│ Entity   │ Entity   │ Entity    │
+---------------------------------+
```

The chunked design provides predictable allocation behavior while allowing component storage to grow as the simulation grows.

It also provides a foundation for efficient iteration over component data.

## Entity-to-Component Mapping

The ECS must be able to answer two questions efficiently:

1. Where is an entity's component data?
2. Which entity owns a particular component entry?

The storage therefore maintains location information for component entries.

Conceptually:

```text
Entity
   │
Location
+-- Chunk
+-- Offset
+-- Generation
+-- Present
   │
Component data
```

This allows the ECS to locate component data without embedding the data directly inside the entity.

## Adding Components

Components are associated with entities through the World:

```cpp
world.add_component(
    entity,
    Position{0.0f, 0.0f, 0.0f}
);
```

The component is placed into the appropriate component storage.

Systems do not need to know how that storage is allocated or how the entity-to-storage mapping is maintained.

## Queries

Queries provide systems with access to the component combinations they require.

A system declares its data requirements through its `Query` type.

For example:

```cpp
using Query = conductor::Query<
    conductor::Read<Velocity>,
    conductor::Write<Position>
>;
```

This means the system requires:

* read access to `Velocity`
* write access to `Position`

The query is then bound to the World by the runtime when the system is registered.

During execution, the system iterates over the matching entities:

```cpp
for (auto [velocity, position] : ctx.query()) {
    position.x += velocity.x;
    position.y += velocity.y;
    position.z += velocity.z;
}
```

The system does not manually search the entity manager or component storages.

The query provides the relevant data directly.

## Read and Write Access

Component access is explicitly declared as either read or write.

```text
Read<T>
Write<T>
```

For example:

```cpp
Query<
    Read<Velocity>,
    Write<Position>
>
```

describes a system that reads velocity and modifies position.

This distinction is important beyond the query itself.

The Scheduler uses system data access information to reason about dependencies and potential conflicts between systems.

For example:

```text
System A
Read<Position>

System B
Write<Position>
```

These systems cannot necessarily execute independently because System B modifies data that System A accesses.

Likewise:

```text
System A
Write<Position>

System B
Write<Position>
```

represents a write/write conflict that requires an execution dependency.

The ECS therefore provides information that becomes part of the scheduler's execution model.

## ECS and Scheduling

One of the important architectural properties of Conductor is that the ECS and Scheduler are not completely independent systems.

The ECS describes **what data a system accesses**.

The Scheduler uses that information to determine **how systems may execute relative to one another**.

Conceptually:

```text
System
  │
  │ declares
  |
Query
  │
  +-- Read<Position>
  +-- Write<Velocity>
  +-- ...
  │
Scheduler
  │
  +-- dependency inference
  +-- conflict detection
  +-- execution ordering
```

This allows dependencies to be derived from actual data access rather than requiring every dependency to be manually specified.

Explicit dependencies can still be used when the simulation has ordering requirements that cannot be inferred from component access alone.

## Query Matching

A query describes a combination of component types.

For example:

```cpp
Query<
    Read<Position>,
    Read<Velocity>
>
```

matches entities that contain both `Position` and `Velocity`.

An entity containing only:

```text
Position
```

does not match.

An entity containing:

```text
Position
Velocity
Battery
```

does match.

The ECS therefore allows systems to operate on subsets of the simulation without requiring systems to maintain their own lists of entities.

## Structural Changes

Adding or removing components changes the structure of an entity.

These operations are therefore treated differently from ordinary component data modification.

During system execution, structural operations are deferred through the World-owned `CommandQueue`.

For example:

```cpp
ctx.commands().add_component(
    entity,
    Battery{100.0f}
);
```

The operation is queued rather than immediately modifying the component storage being used by the current execution.

Conceptually:

```text
System
  │
  +-- Read/write component data immediately
  │
  +-- Structural change
          │
    CommandQueue
          │
    Synchronization
          │
    Component storage updated
```

This distinction is important because changing storage while systems are iterating over it could invalidate iteration state and complicate parallel execution.

## Data Modification vs. Structural Modification

The ECS therefore distinguishes between two kinds of operations.

### Data modification

Changing the value of an existing component:

```cpp
position.x += velocity.x * dt;
```

This occurs directly through the system's query.

### Structural modification

Changing which components an entity possesses:

```cpp
add_component(entity, Battery{});
remove_component<Velocity>(entity);
```

These operations are deferred through the command queue.

This distinction provides a stable view of the ECS during system execution.

## Deferred Entities

Conductor also supports deferred entity references for commands involving entities that are created during simulation.

The deferred reference represents an entity that will become valid after the command queue is processed.

Conceptually:

```text
Create Entity
      │
Deferred Entity
      │
      │ used by later commands
      |
Command application
      │
Actual Entity
```

This allows systems to construct new simulation state without requiring immediate structural mutation.

## Component Storage Registry

The World maintains a `ComponentStorageRegistry` that associates component type IDs with their concrete storage instances.

The registry provides the runtime with a common interface for operations such as:

* locating storage
* adding components
* removing compo
