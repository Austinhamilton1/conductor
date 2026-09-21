# Engine Lifecycle

Conductor has a deliberately defined lifecycle for constructing, configuring, executing, and shutting down a simulation.

The lifecycle establishes when the simulation may be modified, when systems and plugins must remain alive, and when resources can safely be destroyed.

## High-Level Lifecycle

The complete lifecycle is:

```text
Application construction
        │
Plugin loading / configuration
        │
World configuration
        │
World::finalize()
        │
Simulation
        │
Plugin shutdown
        │
World teardown
        │
Plugin destruction
        │
Dynamic library unload
```

The important boundary is `World::finalize()`.

Before finalization, the simulation is being constructed. After finalization, the simulation is being executed.

## 1. Application Construction

The application creates the runtime infrastructure required to execute a simulation.

This includes the World and the services responsible for application-level execution.

At this point, no simulation-specific behavior needs to exist yet.

```text
Application
    │
    +-- World
```

## 2. Plugin Loading and Configuration

Plugins are loaded before simulation begins.

A plugin is constructed from its shared library and initialized with a reference to the World:

```cpp
plugin.initialize(world);
```

During initialization, a plugin may configure the simulation by:

* registering components
* registering systems
* creating initial entities
* adding component data
* configuring simulation-specific behavior

A plugin should perform structural configuration during initialization rather than attempting to modify the World later during active simulation.

## 3. World Configuration

After plugins are initialized, the World contains the simulation's initial structure.

For example:

```text
World
+-- Component types
│   +-- Position
│   +-- Velocity
│   +-- Battery
│
+-- Entities
│   +-- Drone 1
│   +-- Drone 2
│   +-- Drone 3
│
+-- Systems
    +-- Navigation
    +-- Movement
    +-- Battery
```

The exact contents are determined by the application and its plugins.

The kernel itself does not require any particular domain-specific components or systems.

## 4. Finalization

Once configuration is complete:

```cpp
world.finalize();
```

This establishes the runtime configuration.

After this point, structural configuration operations are no longer permitted.

The finalization boundary is important because the scheduler, component storage, and other runtime structures can now be treated as established simulation infrastructure.

```text
CONFIGURATION

register components
register systems
create entities
configure plugins
      │
      │
World::finalize()
      │
─────────────────────────────────────────────────
EXECUTION
      │
run systems
process simulation
queue commands
advance time
apply structural changes
─────────────────────────────────────────────────
```

## 5. Simulation

During simulation, the application repeatedly advances the World.

Conceptually:

```cpp
while (running) {
    world.update(delta_seconds);
}
```

The World delegates system execution to the Scheduler.

Systems access simulation state through their queries and runtime context.

Structural modifications are deferred through the command queue rather than being applied arbitrarily during system execution.

This creates a predictable execution model:

```text
Simulation step
      │
Scheduler executes systems
      │
Systems enqueue structural changes
      │
Synchronization / command application
      │
Next simulation state
```

## 6. Plugin Lifetime During Simulation

A plugin must remain loaded while its systems are active.

This is particularly important for dynamically loaded plugins.

A plugin may define:

* component types
* system implementations
* storage callbacks
* system state
* other code referenced by the simulation

Those objects and functions belong to the plugin's shared library.

Therefore, the shared library cannot be unloaded while the World still contains objects or callbacks originating from that library.

The intended lifetime is:

```text
Plugin loaded
     │
Plugin initialized
     │
Systems active
     │
Simulation running
     │
Plugin shutdown
     │
Plugin objects destroyed
     │
Library unloaded
```

Conductor does not support hot unloading or reloading of plugins during an active simulation.

## 7. Plugin Shutdown

When simulation execution ends, plugins receive their shutdown callback:

```cpp
plugin.shutdown(world);
```

Shutdown provides the plugin with an opportunity to perform application-defined cleanup while the World is still available.

The World must therefore remain valid during plugin shutdown.

Conceptually:

```text
Simulation stops
      │
Plugin::shutdown(World&)
      │
World still valid
```

Shutdown is distinct from destruction.

A plugin should use `shutdown()` for simulation/application lifecycle cleanup, while its destructor handles destruction of the plugin object itself.

## 8. World Teardown

After plugin shutdown, the World can be torn down.

World-owned systems and simulation state are destroyed as part of normal object lifetime.

This ordering is significant for dynamically loaded plugins: any objects that reference plugin-defined code must be destroyed while the plugin library is still loaded.

The intended relationship is:

```text
Plugin shutdown
      │
World/system teardown
      │
Plugin object destruction
      │
dlclose()
```

The exact implementation may vary as the runtime evolves, but this lifetime invariant must be preserved.

## System Handle Lifetime

Systems have an additional lifetime consideration.

Registering a system returns a `Scheduler::SystemHandle`.

The handle represents the lifetime of the registered system.

Conceptually:

```text
add_system()
     │
System registered
     │
     │  SystemHandle alive
     │
System executes
     │
SystemHandle destroyed
     │
System removed
```

A plugin that owns a system should therefore retain its handle for as long as that system is intended to exist.

For example:

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

A temporary handle should not be used when the system is intended to remain registered for the lifetime of the simulation.

This makes system ownership explicit and prevents accidental registration followed immediately by destruction of the corresponding system.

## Lifecycle Invariants

The engine lifecycle is built around several important invariants.

### Configuration precedes execution

Structural simulation configuration must be completed before:

```cpp
world.finalize();
```

### Finalization precedes simulation

The World must be finalized before active simulation updates begin.

### Plugins outlive their systems

A dynamically loaded plugin must remain loaded while the World contains systems or callbacks implemented by that plugin.

### World outlives plugin shutdown

`Plugin::shutdown(World&)` requires the World to still be valid.

### Plugin libraries outlive plugin-defined objects

Plugin-defined systems, storage callbacks, and other objects must be destroyed before their shared library is unloaded.

### System handles must outlive their systems

A `SystemHandle` must remain alive for as long as its associated system is intended to remain registered.

## Complete Lifecycle

Putting these rules together:

```text
+---------------------------+
│ Application Construction  │
+---------------------------+
              │
+---------------------------+
│ Plugin Loading            │
│                           │
│ > Load shared library     │
│ > Construct plugin        │
│ > Initialize plugin       │
+---------------------------+
              │
+---------------------------+
│ World Configuration       │
│                           │
│ > Register components     │
│ > Register systems        │
│ > Create entities         │
│ > Configure simulation    │
+---------------------------+
              │
        World::finalize()
              │
+---------------------------+
│ Simulation                │
│                           │
│ > Advance time            │
│ > Execute systems         │
│ > Queue commands          │
│ > Apply structural work   │
+---------------------------+
              │
+---------------------------+
│ Plugin Shutdown           │
│                           │
│ Plugin::shutdown(world)   │
+---------------------------+
              │
+---------------------------+
│ World / System Teardown   │
+---------------------------+
              │
+---------------------------+
│ Plugin Destruction        │
+---------------------------+
              │
+---------------------------+
│ Shared Library Unload     │
│                           │
│ dlclose()                 │
+---------------------------+
```

## Design Principle

Conductor treats lifecycle as part of the architecture rather than as incidental object management.

The key idea is simple:

> **Configure first, finalize once, simulate, then tear down in dependency order.**

This provides a stable foundation for plugins, scheduling, deferred commands, and future execution backends without requiring the kernel to know anything about the domain being simulated.
