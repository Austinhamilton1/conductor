#include <catch2/catch_test_macros.hpp>

#include "commands/CommandQueue.h"
#include "ecs/ComponentRegistry.h"
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


// =============================================================================
// Entity creation
// =============================================================================

TEST_CASE("CommandQueue defers entity creation") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    const auto deferred =
        commands.create_entity();

    REQUIRE(deferred.valid());
    REQUIRE(commands.size() == 1);

    REQUIRE_THROWS_AS(
        commands.resolve(deferred),
        std::out_of_range
    );

    commands.apply(
        entities,
        storages
    );

    const auto entity =
        commands.resolve(deferred);

    REQUIRE(entity.valid());
    REQUIRE(entities.alive(entity));
}


// =============================================================================
// Deferred entity resolution
// =============================================================================

TEST_CASE("CommandQueue resolves deferred entities after apply") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    const auto first =
        commands.create_entity();

    const auto second =
        commands.create_entity();

    REQUIRE_THROWS_AS(
        commands.resolve(first),
        std::out_of_range
    );

    REQUIRE_THROWS_AS(
        commands.resolve(second),
        std::out_of_range
    );

    commands.apply(
        entities,
        storages
    );

    const auto first_entity =
        commands.resolve(first);

    const auto second_entity =
        commands.resolve(second);

    REQUIRE(first_entity.valid());
    REQUIRE(second_entity.valid());

    REQUIRE(first_entity != second_entity);

    REQUIRE(entities.alive(first_entity));
    REQUIRE(entities.alive(second_entity));
}


// =============================================================================
// Entity destruction
// =============================================================================

TEST_CASE("CommandQueue defers entity destruction") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    const Entity entity =
        entities.create();

    REQUIRE(entities.alive(entity));

    commands.destroy_entity(entity);

    REQUIRE(commands.size() == 1);

    // Still alive before command application.
    REQUIRE(entities.alive(entity));

    commands.apply(
        entities,
        storages
    );

    REQUIRE_FALSE(
        entities.alive(entity)
    );
}


// =============================================================================
// Deferred entity destruction
// =============================================================================

TEST_CASE("CommandQueue can destroy a deferred entity") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    const auto deferred =
        commands.create_entity();

    commands.destroy_entity(deferred);

    commands.apply(
        entities,
        storages
    );

    const auto entity =
        commands.resolve(deferred);

    REQUIRE_FALSE(
        entities.alive(entity)
    );
}


// =============================================================================
// Component addition
// =============================================================================

TEST_CASE("CommandQueue defers component addition") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    auto& position_storage =
        storages.create<Position>();

    const Entity entity =
        entities.create();

    const Position position{
        1.0f,
        2.0f,
        3.0f
    };

    commands.add_component(
        entity,
        position
    );

    REQUIRE(commands.size() == 1);

    REQUIRE_FALSE(
        position_storage.contains(entity)
    );

    commands.apply(
        entities,
        storages
    );

    REQUIRE(
        position_storage.contains(entity)
    );

    const auto& actual =
        position_storage.get(entity);

    REQUIRE(actual->x == 1.0f);
    REQUIRE(actual->y == 2.0f);
    REQUIRE(actual->z == 3.0f);
}


// =============================================================================
// Component data is copied
// =============================================================================

TEST_CASE("CommandQueue copies component data when queued") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    auto& position_storage =
        storages.create<Position>();

    const Entity entity =
        entities.create();

    Position position{
        1.0f,
        2.0f,
        3.0f
    };

    commands.add_component(
        entity,
        position
    );

    // Modify the original after queuing.
    position.x = 100.0f;
    position.y = 200.0f;
    position.z = 300.0f;

    commands.apply(
        entities,
        storages
    );

    const auto& actual =
        position_storage.get(entity);

    REQUIRE(actual->x == 1.0f);
    REQUIRE(actual->y == 2.0f);
    REQUIRE(actual->z == 3.0f);
}


// =============================================================================
// Deferred entity + component
// =============================================================================

TEST_CASE("CommandQueue can add a component to a deferred entity") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    auto& position_storage =
        storages.create<Position>();

    const auto deferred =
        commands.create_entity();

    commands.add_component(
        deferred,
        Position{
            10.0f,
            20.0f,
            30.0f
        }
    );

    REQUIRE(commands.size() == 2);

    commands.apply(
        entities,
        storages
    );

    const Entity entity =
        commands.resolve(deferred);

    REQUIRE(entities.alive(entity));
    REQUIRE(position_storage.contains(entity));

    const auto& position =
        position_storage.get(entity);

    REQUIRE(position->x == 10.0f);
    REQUIRE(position->y == 20.0f);
    REQUIRE(position->z == 30.0f);
}


// =============================================================================
// Multiple components
// =============================================================================

TEST_CASE("CommandQueue can add multiple components to an entity") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    auto& positions =
        storages.create<Position>();

    auto& velocities =
        storages.create<Velocity>();

    const Entity entity =
        entities.create();

    commands.add_component(
        entity,
        Position{1.0f, 2.0f, 3.0f}
    );

    commands.add_component(
        entity,
        Velocity{4.0f, 5.0f}
    );

    commands.apply(
        entities,
        storages
    );

    REQUIRE(positions.contains(entity));
    REQUIRE(velocities.contains(entity));

    const auto& position =
        positions.get(entity);

    const auto& velocity =
        velocities.get(entity);

    REQUIRE(position->x == 1.0f);
    REQUIRE(position->y == 2.0f);
    REQUIRE(position->z == 3.0f);

    REQUIRE(velocity->x == 4.0f);
    REQUIRE(velocity->y == 5.0f);
}


// =============================================================================
// Component removal
// =============================================================================

TEST_CASE("CommandQueue defers component removal") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    auto& positions =
        storages.create<Position>();

    const Entity entity =
        entities.create();

    positions.add(
        entity,
        Position{1.0f, 2.0f, 3.0f}
    );

    REQUIRE(positions.contains(entity));

    commands.remove_component<Position>(
        entity
    );

    REQUIRE(
        positions.contains(entity)
    );

    commands.apply(
        entities,
        storages
    );

    REQUIRE_FALSE(
        positions.contains(entity)
    );
}


// =============================================================================
// Deferred entity + component removal
// =============================================================================

TEST_CASE("CommandQueue can remove components from deferred entity") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    auto& positions =
        storages.create<Position>();

    const auto deferred =
        commands.create_entity();

    commands.add_component(
        deferred,
        Position{1.0f, 2.0f, 3.0f}
    );

    commands.apply(
        entities,
        storages
    );

    const Entity entity =
        commands.resolve(deferred);

    REQUIRE(
        positions.contains(entity)
    );

    commands.remove_component<Position>(
        deferred
    );

    REQUIRE(
        positions.contains(entity)
    );

    commands.apply(
        entities,
        storages
    );

    REQUIRE_FALSE(
        positions.contains(entity)
    );
}


// =============================================================================
// Entity commands execute before component commands
// =============================================================================

TEST_CASE("CommandQueue applies entity commands before component commands") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    auto& positions =
        storages.create<Position>();

    const auto deferred =
        commands.create_entity();

    commands.add_component(
        deferred,
        Position{1.0f, 2.0f, 3.0f}
    );

    commands.apply(
        entities,
        storages
    );

    const Entity entity =
        commands.resolve(deferred);

    REQUIRE(entities.alive(entity));
    REQUIRE(positions.contains(entity));
}


// =============================================================================
// Multiple deferred entities
// =============================================================================

TEST_CASE("CommandQueue supports multiple deferred entities with components") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    auto& positions =
        storages.create<Position>();

    const auto first =
        commands.create_entity();

    const auto second =
        commands.create_entity();

    commands.add_component(
        first,
        Position{1.0f, 2.0f, 3.0f}
    );

    commands.add_component(
        second,
        Position{4.0f, 5.0f, 6.0f}
    );

    commands.apply(
        entities,
        storages
    );

    const Entity first_entity =
        commands.resolve(first);

    const Entity second_entity =
        commands.resolve(second);

    REQUIRE(entities.alive(first_entity));
    REQUIRE(entities.alive(second_entity));

    REQUIRE(positions.contains(first_entity));
    REQUIRE(positions.contains(second_entity));

    REQUIRE(
        positions.get(first_entity)->x == 1.0f
    );

    REQUIRE(
        positions.get(second_entity)->x == 4.0f
    );
}


// =============================================================================
// Clear
// =============================================================================

TEST_CASE("CommandQueue clear removes queued commands") {
    CommandQueue commands;

    commands.create_entity();

    REQUIRE_FALSE(commands.empty());
    REQUIRE(commands.size() == 1);

    commands.clear();

    REQUIRE(commands.empty());
    REQUIRE(commands.size() == 0);
}


// =============================================================================
// Clear invalidates deferred entities
// =============================================================================

TEST_CASE("CommandQueue clear invalidates deferred entities") {
    CommandQueue commands;

    const auto deferred =
        commands.create_entity();

    commands.clear();

    REQUIRE_THROWS_AS(
        commands.resolve(deferred),
        std::out_of_range
    );
}


// =============================================================================
// Apply clears commands
// =============================================================================

TEST_CASE("CommandQueue apply clears commands") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    commands.create_entity();

    REQUIRE(commands.size() == 1);

    commands.apply(
        entities,
        storages
    );

    REQUIRE(commands.empty());
    REQUIRE(commands.size() == 0);
}


// =============================================================================
// Deferred entities remain resolvable after apply
// =============================================================================

TEST_CASE("CommandQueue retains deferred resolutions after apply") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    const auto deferred =
        commands.create_entity();

    commands.apply(
        entities,
        storages
    );

    const Entity entity =
        commands.resolve(deferred);

    REQUIRE(entity.valid());

    // The command list has been cleared, but the resolution remains.
    REQUIRE(commands.empty());

    REQUIRE(entities.alive(entity));
}


// =============================================================================
// Invalid DeferredEntity
// =============================================================================

TEST_CASE("CommandQueue rejects invalid DeferredEntity") {
    CommandQueue commands;

    const DeferredEntity invalid{};

    REQUIRE_THROWS_AS(
        commands.resolve(invalid),
        std::out_of_range
    );
}


// =============================================================================
// DeferredEntity from another queue
// =============================================================================

TEST_CASE("CommandQueue rejects DeferredEntity from another queue") {
    EntityManager entities;
    ComponentStorageRegistry storages;

    CommandQueue first;
    CommandQueue second;

    const auto deferred =
        first.create_entity();

    REQUIRE_THROWS_AS(
        second.resolve(deferred),
        std::out_of_range
    );

    // Make sure the first queue still works normally.
    first.apply(
        entities,
        storages
    );

    REQUIRE(
        first.resolve(deferred).valid()
    );
}


// =============================================================================
// Missing storage
// =============================================================================

TEST_CASE("CommandQueue rejects adding a component without storage") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    const Entity entity =
        entities.create();

    commands.add_component(
        entity,
        Position{1.0f, 2.0f, 3.0f}
    );

    REQUIRE_THROWS_AS(
        commands.apply(
            entities,
            storages
        ),
        std::logic_error
    );
}


// =============================================================================
// Missing component during removal
// =============================================================================

TEST_CASE("CommandQueue rejects removing a component without storage") {
    EntityManager entities;
    ComponentStorageRegistry storages;
    CommandQueue commands;

    const Entity entity =
        entities.create();

    commands.remove_component<Position>(
        entity
    );

    REQUIRE_THROWS_AS(
        commands.apply(
            entities,
            storages
        ),
        std::logic_error
    );
}