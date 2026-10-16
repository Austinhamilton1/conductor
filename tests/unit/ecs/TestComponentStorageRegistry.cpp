#include <catch2/catch_test_macros.hpp>

#include "ecs/ComponentStorageRegistry.h"
#include "ecs/EntityManager.h"

using namespace conductor;

namespace {

    struct Position {
        float x;
        float y;
        float z;
    };

    struct Velocity {
        float x;
        float y;
    };

    struct Health {
        int value;
    };

}


// -----------------------------------------------------------------------------
// Creation
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry creates component storage", "[ecs]") {
    ComponentStorageRegistry registry;

    auto& storage =
        registry.create<Position>();

    REQUIRE(registry.size() == 1);
    REQUIRE(registry.contains<Position>());
    REQUIRE(&registry.get<Position>() == &storage);
}


// -----------------------------------------------------------------------------
// Multiple component types
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry stores multiple component types", "[ecs]") {
    ComponentStorageRegistry registry;

    auto& position =
        registry.create<Position>();

    auto& velocity =
        registry.create<Velocity>();

    REQUIRE(registry.size() == 2);

    REQUIRE(registry.contains<Position>());
    REQUIRE(registry.contains<Velocity>());

    REQUIRE(&registry.get<Position>() == &position);
    REQUIRE(&registry.get<Velocity>() == &velocity);

    REQUIRE(&position !=
            reinterpret_cast<void*>(&velocity));
}


// -----------------------------------------------------------------------------
// Component IDs map to storage entries
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry maps component IDs to storage", "[ecs]") {
    ComponentStorageRegistry registry;

    auto position_id =
        ComponentRegistry::register_component<Position>();

    auto velocity_id =
        ComponentRegistry::register_component<Velocity>();

    auto& position =
        registry.create<Position>();

    auto& velocity =
        registry.create<Velocity>();

    auto& position_entry =
        registry.entry(position_id);

    auto& velocity_entry =
        registry.entry(velocity_id);

    REQUIRE(position_entry.valid());
    REQUIRE(velocity_entry.valid());

    REQUIRE(position_entry.storage ==
            static_cast<void*>(&position));

    REQUIRE(velocity_entry.storage ==
            static_cast<void*>(&velocity));
}


// -----------------------------------------------------------------------------
// Lookup by ComponentTypeId
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry supports lookup by component ID", "[ecs]") {
    ComponentStorageRegistry registry;

    auto position_id =
        ComponentRegistry::register_component<Position>();

    auto& position =
        registry.create<Position>();

    auto& entry =
        registry.entry(position_id);

    REQUIRE(entry.storage ==
            static_cast<void*>(&position));
}


// -----------------------------------------------------------------------------
// Missing storage
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry reports missing storage", "[ecs]") {
    ComponentStorageRegistry registry;

    REQUIRE_FALSE(
        registry.contains<Position>()
    );
}

TEST_CASE("ComponentStorageRegistry get throws when storage does not exist", "[ecs]") {
    ComponentStorageRegistry registry;

    REQUIRE_THROWS_AS(
        registry.get<Position>(),
        std::logic_error
    );
}


// -----------------------------------------------------------------------------
// Invalid IDs
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry rejects invalid component IDs", "[ecs]") {
    ComponentStorageRegistry registry;

    const ComponentTypeId invalid_id = 999999;

    REQUIRE_FALSE(
        registry.contains(invalid_id)
    );

    REQUIRE_THROWS_AS(
        registry.entry(invalid_id),
        std::out_of_range
    );
}


// -----------------------------------------------------------------------------
// Duplicate storage
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry rejects duplicate storage", "[ecs]") {
    ComponentStorageRegistry registry;

    registry.create<Position>();

    REQUIRE_THROWS_AS(
        registry.create<Position>(),
        std::logic_error
    );

    REQUIRE(registry.size() == 1);
}


// -----------------------------------------------------------------------------
// Destruction
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry destroys component storage", "[ecs]") {
    ComponentStorageRegistry registry;

    registry.create<Position>();

    REQUIRE(registry.contains<Position>());
    REQUIRE(registry.size() == 1);

    registry.destroy<Position>();

    REQUIRE_FALSE(registry.contains<Position>());
    REQUIRE(registry.size() == 0);
}


// -----------------------------------------------------------------------------
// Destruction by ID
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry destroys storage by component ID", "[ecs]") {
    ComponentStorageRegistry registry;

    const auto position_id =
        ComponentRegistry::register_component<Position>();

    registry.create<Position>();

    REQUIRE(registry.contains(position_id));

    registry.destroy(position_id);

    REQUIRE_FALSE(registry.contains(position_id));
    REQUIRE(registry.size() == 0);
}


// -----------------------------------------------------------------------------
// Destroying missing storage
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry rejects destroying missing storage", "[ecs]") {
    ComponentStorageRegistry registry;

    REQUIRE_THROWS_AS(
        registry.destroy<Position>(),
        std::out_of_range
    );
}


// -----------------------------------------------------------------------------
// Recreate after destruction
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry can recreate destroyed storage", "[ecs]") {
    ComponentStorageRegistry registry;

    auto& first =
        registry.create<Position>();

    REQUIRE(registry.contains<Position>());

    registry.destroy<Position>();

    REQUIRE_FALSE(registry.contains<Position>());

    auto& second =
        registry.create<Position>();

    REQUIRE(registry.contains<Position>());
    REQUIRE(registry.size() == 1);
    REQUIRE(&registry.get<Position>() == &second);
}


// -----------------------------------------------------------------------------
// Const access
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry supports const storage access", "[ecs]") {
    ComponentStorageRegistry registry;

    registry.create<Position>();

    const auto& const_registry =
        registry;

    const auto& storage =
        const_registry.get<Position>();

    REQUIRE(&storage ==
            &registry.get<Position>());
}


// -----------------------------------------------------------------------------
// Registry capacity
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry capacity tracks component ID space", "[ecs]") {
    ComponentStorageRegistry registry;

    const auto position_id =
        ComponentRegistry::register_component<Position>();

    registry.create<Position>();

    REQUIRE(
        registry.capacity() >
        static_cast<size_t>(position_id)
    );
}


// -----------------------------------------------------------------------------
// Empty registry
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry starts empty", "[ecs]") {
    ComponentStorageRegistry registry;

    REQUIRE(registry.size() == 0);
    REQUIRE(registry.capacity() == 0);
    REQUIRE_FALSE(registry.contains<Position>());
    REQUIRE_FALSE(registry.contains<Velocity>());
}

// -----------------------------------------------------------------------------
// Registry updates
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry creates valid add and remove operations", "[ecs]") {
    ComponentStorageRegistry registry;

    auto& storage =
        registry.create<Position>();

    const auto id =
        ComponentRegistry::register_component<Position>();

    auto& entry =
        registry.entry(id);

    REQUIRE(entry.valid());
    REQUIRE(entry.add != nullptr);
    REQUIRE(entry.remove != nullptr);

    const Entity entity{0, 0};

    const Position position{
        1.0f,
        2.0f,
        3.0f
    };

    entry.add(
        entry.storage,
        entity,
        &position
    );

    entry.flush(entry.storage);

    REQUIRE(storage.contains(entity));

    entry.remove(
        entry.storage,
        entity
    );

    entry.flush(entry.storage);

    REQUIRE_FALSE(storage.contains(entity));
}

TEST_CASE("ComponentStorageRegistry add operation preserves component data", "[ecs]") {
    ComponentStorageRegistry registry;

    auto& storage =
        registry.create<Position>();

    const auto id =
        ComponentRegistry::register_component<Position>();

    auto& entry =
        registry.entry(id);

    const Entity entity{0, 0};

    const Position expected{
        10.0f,
        20.0f,
        30.0f
    };

    entry.add(
        entry.storage,
        entity,
        &expected
    );

    entry.flush(entry.storage);

    const Position& actual =
        *storage.get(entity);

    REQUIRE(actual.x == expected.x);
    REQUIRE(actual.y == expected.y);
    REQUIRE(actual.z == expected.z);
}

TEST_CASE("Destroying an entity removes all components", "[ecs]") {
    EntityManager manager;
    ComponentStorageRegistry storages;

    auto& positions = storages.create<Position>();
    auto& velocities = storages.create<Velocity>();
    auto& batteries = storages.create<Health>();

    Entity entity = manager.create();

    positions.add(entity, Position{1.0, 2.0});
    velocities.add(entity, Velocity{3.0, 4.0});
    batteries.add(entity, Health{100});
    
    positions.flush_additions();
    velocities.flush_additions();
    batteries.flush_additions();

    REQUIRE(positions.contains(entity));
    REQUIRE(velocities.contains(entity));
    REQUIRE(batteries.contains(entity));

    manager.destroy(entity);
    storages.remove_entity(entity);
    storages.flush();

    REQUIRE_FALSE(positions.contains(entity));
    REQUIRE_FALSE(velocities.contains(entity));
    REQUIRE_FALSE(batteries.contains(entity));
}