#include <catch2/catch_test_macros.hpp>

#include "simulation/World.h"
#include "ecs/SystemContext.h"

namespace {
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

    struct MovementSystem {
        using Query = conductor::Query<
            conductor::Read<Velocity>,
            conductor::Write<Position>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT> ctx) {
            for(auto [velocity, position] : ctx.query()) {
                position.x += velocity.x * ctx.delta_seconds();
                position.y += velocity.y * ctx.delta_seconds();
                position.z += velocity.z * ctx.delta_seconds();
            }
        }
    };
}

TEST_CASE("World can be constructed", "[simulation]") {
    conductor::Random rand;
    conductor::World world(rand);

    REQUIRE_FALSE(false);
}

TEST_CASE("World creates entities", "[simulation]") {
    conductor::Random rand;
    conductor::World world(rand);

    auto entity1 = world.create_entity();
    auto entity2 = world.create_entity();

    REQUIRE(entity1 != entity2);
    REQUIRE(world.entities().alive(entity1));
    REQUIRE(world.entities().alive(entity2));
}

TEST_CASE("World destroys entities", "[simulation]") {
    conductor::Random rand;
    conductor::World world(rand);

    auto entity = world.create_entity();

    REQUIRE(world.entities().alive(entity));

    world.destroy_entity(entity);

    REQUIRE_FALSE(world.entities().alive(entity));
}

TEST_CASE("World registers components", "[simulation]") {
    conductor::Random rand;
    conductor::World world(rand);

    world.register_component<Position>();
    world.register_component<Velocity>();

    auto position_info = world.components().info<Position>();
    auto velocity_info = world.components().info<Velocity>();

    REQUIRE(position_info.id != velocity_info.id);
}

TEST_CASE("World creates component storage", "[simulation]") {
    conductor::Random rand;
    conductor::World world(rand);

    world.register_component<Position>();

    auto& storage = world.storage();

    REQUIRE(storage.contains<Position>());
}

TEST_CASE("World adds components to entities", "[simulation]") {
    conductor::Random rand;
    conductor::World world(rand);

    world.register_component<Position>();

    auto entity = world.create_entity();

    Position position{
        1.0f,
        2.0f,
        3.0f
    };

    world.add_component(entity, position);

    auto& storage = world.storage().get<Position>();

    REQUIRE(storage.contains(entity));

    auto result = storage.get(entity);

    REQUIRE(result->x == 1.0f);
    REQUIRE(result->y == 2.0f);
    REQUIRE(result->z == 3.0f);
}

TEST_CASE("World can add a system", "[simulation]") {
    conductor::Random rand;
    conductor::World world(rand);

    world.register_component<Position>();
    world.register_component<Velocity>();

    auto system = world.add_system(
        "Movement",
        60.0,
        MovementSystem{}
    );

    REQUIRE(system.valid());
}

TEST_CASE("World updates systems", "[simulation]") {
    conductor::Random rand;
    conductor::World world(rand);

    world.register_component<Position>();
    world.register_component<Velocity>();

    auto entity = world.create_entity();

    Position position{
        0.0f,
        0.0f,
        0.0f
    };

    Velocity velocity{
        1.0f,
        2.0f,
        3.0f
    };

    world.add_component(entity, position);
    world.add_component(entity, velocity);

    auto handle = world.add_system(
        "Movement",
        1.0,
        MovementSystem{}
    );

    world.finalize();

    world.update(1.0);

    auto& positions = world.storage().get<Position>();
    auto result = positions.get(entity);

    REQUIRE(result->x == 1.0f);
    REQUIRE(result->y == 2.0f);
    REQUIRE(result->z == 3.0f);
}

TEST_CASE("World prevents component registration after finalization", "[simulation]") {
    conductor::Random rand;
    conductor::World world(rand);

    world.register_component<Position>();
    world.finalize();

    REQUIRE_THROWS_AS(
        world.register_component<Velocity>(),
        std::logic_error
    );
}

TEST_CASE("World prevents adding systems after finalization", "[simulation]") {
    conductor::Random rand;
    conductor::World world(rand);

    world.register_component<Position>();
    world.register_component<Velocity>();

    world.finalize();

    REQUIRE_THROWS_AS(
        world.add_system(
            "Movement",
            60.0,
            MovementSystem{}
        ),
        std::logic_error
    );
}

TEST_CASE("World prevents entity creation after finalization", "[simulation]") {
    conductor::Random rand;
    conductor::World world(rand);

    world.finalize();

    REQUIRE_THROWS_AS(
        world.create_entity(),
        std::logic_error
    );
}

TEST_CASE("World prevents entity destruction after finalization", "[simulation]") {
    conductor::Random rand;
    conductor::World world(rand);

    auto entity = world.create_entity();

    world.finalize();

    REQUIRE_THROWS_AS(
        world.destroy_entity(entity),
        std::logic_error
    );
}