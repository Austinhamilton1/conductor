#include <catch2/catch_test_macros.hpp>

#include "ComponentStorageRegistry.h"

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

TEST_CASE("ComponentStorageRegistry creates component storage") {
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

TEST_CASE("ComponentStorageRegistry stores multiple component types") {
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

TEST_CASE("ComponentStorageRegistry maps component IDs to storage") {
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

TEST_CASE("ComponentStorageRegistry supports lookup by component ID") {
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

TEST_CASE("ComponentStorageRegistry reports missing storage") {
    ComponentStorageRegistry registry;

    REQUIRE_FALSE(
        registry.contains<Position>()
    );
}

TEST_CASE("ComponentStorageRegistry get throws when storage does not exist") {
    ComponentStorageRegistry registry;

    REQUIRE_THROWS_AS(
        registry.get<Position>(),
        std::logic_error
    );
}


// -----------------------------------------------------------------------------
// Invalid IDs
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry rejects invalid component IDs") {
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

TEST_CASE("ComponentStorageRegistry rejects duplicate storage") {
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

TEST_CASE("ComponentStorageRegistry destroys component storage") {
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

TEST_CASE("ComponentStorageRegistry destroys storage by component ID") {
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

TEST_CASE("ComponentStorageRegistry rejects destroying missing storage") {
    ComponentStorageRegistry registry;

    REQUIRE_THROWS_AS(
        registry.destroy<Position>(),
        std::out_of_range
    );
}


// -----------------------------------------------------------------------------
// Recreate after destruction
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorageRegistry can recreate destroyed storage") {
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

TEST_CASE("ComponentStorageRegistry supports const storage access") {
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

TEST_CASE("ComponentStorageRegistry capacity tracks component ID space") {
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

TEST_CASE("ComponentStorageRegistry starts empty") {
    ComponentStorageRegistry registry;

    REQUIRE(registry.size() == 0);
    REQUIRE(registry.capacity() == 0);
    REQUIRE_FALSE(registry.contains<Position>());
    REQUIRE_FALSE(registry.contains<Velocity>());
}