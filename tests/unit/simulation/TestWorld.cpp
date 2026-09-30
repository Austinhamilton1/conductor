#include <catch2/catch_test_macros.hpp>

#include "simulation/World.h"
#include "ecs/SystemContext.h"

namespace {
    struct Position {
        double x;
        double y;
        double z;
    };

    struct Velocity {
        double x;
        double y;
        double z;
    };

    struct Battery {
        double charge;
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
	conductor::SimulationClock clock;
    conductor::World world(rand);

    REQUIRE_FALSE(false);
}

TEST_CASE("World creates entities", "[simulation]") {
    conductor::Random rand;
	conductor::SimulationClock clock;
    conductor::World world(rand);

    auto entity1 = world.create_entity();
    auto entity2 = world.create_entity();

    REQUIRE(entity1 != entity2);
    REQUIRE(world.entities().alive(entity1));
    REQUIRE(world.entities().alive(entity2));
}

TEST_CASE("World destroys entities", "[simulation]") {
    conductor::Random rand;
	conductor::SimulationClock clock;
    conductor::World world(rand);

    auto entity = world.create_entity();

    REQUIRE(world.entities().alive(entity));

    world.destroy_entity(entity);

    REQUIRE_FALSE(world.entities().alive(entity));
}

TEST_CASE("World registers components", "[simulation]") {
    conductor::Random rand;
	conductor::SimulationClock clock;
    conductor::World world(rand);

    world.register_component<Position>();
    world.register_component<Velocity>();

    auto position_info = world.components().info<Position>();
    auto velocity_info = world.components().info<Velocity>();

    REQUIRE(position_info.id != velocity_info.id);
}

TEST_CASE("World creates component storage", "[simulation]") {
    conductor::Random rand;
	conductor::SimulationClock clock;
    conductor::World world(rand);

    world.register_component<Position>();

    auto& storage = world.storage();

    REQUIRE(storage.contains<Position>());
}

TEST_CASE("World adds components to entities", "[simulation]") {
    conductor::Random rand;
	conductor::SimulationClock clock;
    conductor::World world(rand);

    world.register_component<Position>();

    auto entity = world.create_entity();

    Position position{
        1.0,
        2.0,
        3.0
    };

    world.add_component(entity, position);

    auto& storage = world.storage().get<Position>();

    REQUIRE(storage.contains(entity));

    auto result = storage.get(entity);

    REQUIRE(result->x == 1.0);
    REQUIRE(result->y == 2.0);
    REQUIRE(result->z == 3.0);
}

TEST_CASE("World can add a system", "[simulation]") {
    conductor::Random rand;
	conductor::SimulationClock clock;
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
	conductor::SimulationClock clock;
    conductor::World world(rand);
    conductor::EventBus events;

    conductor::ApplicationContext ctx{
        .random = rand,
        .clock = clock,
        .events = events,
    };

    world.register_component<Position>();
    world.register_component<Velocity>();

    auto entity = world.create_entity();

    Position position{
        0.0,
        0.0,
        0.0
    };

    Velocity velocity{
        1.0,
        2.0,
        3.0
    };

    world.add_component(entity, position);
    world.add_component(entity, velocity);

    auto handle = world.add_system(
        "Movement",
        1.0,
        MovementSystem{}
    );

    world.finalize();

    world.update(1.0, ctx);

    auto result = world.get_component<Position>(entity);

    REQUIRE(result->x == 1.0);
    REQUIRE(result->y == 2.0);
    REQUIRE(result->z == 3.0);
}

TEST_CASE("World prevents component registration after finalization", "[simulation]") {
    conductor::Random rand;
	conductor::SimulationClock clock;
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
	conductor::SimulationClock clock;
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
	conductor::SimulationClock clock;
    conductor::World world(rand);

    world.finalize();

    REQUIRE_THROWS_AS(
        world.create_entity(),
        std::logic_error
    );
}

TEST_CASE("World prevents entity destruction after finalization", "[simulation]") {
    conductor::Random rand;
	conductor::SimulationClock clock;
    conductor::World world(rand);

    auto entity = world.create_entity();

    world.finalize();

    REQUIRE_THROWS_AS(
        world.destroy_entity(entity),
        std::logic_error
    );
}

TEST_CASE("SpawnConfig can initialize a constant component", "[simulation]") {
    conductor::Random random(12345);
	conductor::SimulationClock clock;
    conductor::World world(random);

    world.register_component<Position>();

    conductor::SpawnConfig config;

    config.component<Position>(
        Position{10.0, 20.0, 30.0}
    );

    world.register_spawner("position", config);
    world.spawn_entities(1, "position");

    conductor::Entity entity = world.get_entity(0);

    REQUIRE(world.entities().alive(entity));

    auto position = world.get_component<Position>(entity);

    REQUIRE(position->x == 10.0);
    REQUIRE(position->y == 20.0);
    REQUIRE(position->z == 30.0);
}

TEST_CASE("SpawnConfig can spawn multiple entities", "[simulation]") {
    conductor::Random random(12345);
	conductor::SimulationClock clock;
    conductor::World world(random);

    world.register_component<Position>();

    conductor::SpawnConfig config;

    config.component<Position>(
        Position{10.0, 20.0, 30.0}
    );

    world.register_spawner("position", config);
    world.spawn_entities(200, "position");

    for(std::size_t i = 0; i < 200; ++i) {
        conductor::Entity entity = world.get_entity(i);

        REQUIRE(world.entities().alive(entity));

        auto position = world.get_component<Position>(entity);

        REQUIRE(position->x == 10.0);
        REQUIRE(position->y == 20.0);
        REQUIRE(position->z == 30.0);
    }
}

TEST_CASE("SpawnConfig can generate component values per entity", "[simulation]") {
    conductor::Random random(12345);
	conductor::SimulationClock clock;
    conductor::World world(random);

    world.register_component<Position>();

    conductor::SpawnConfig config;

    config.component<Position>(
        [](const conductor::SpawnContext& context) {
            return Position{
                static_cast<double>(context.index),
                static_cast<double>(context.index * 10),
                static_cast<double>(context.index * 20)
            };
        }
    );

    world.register_spawner("position", config);
    world.spawn_entities(10, "position");

    for(std::size_t i = 0; i < 10; ++i) {
        conductor::Entity entity = world.get_entity(i);

        auto position = world.get_component<Position>(entity);

        REQUIRE(position->x == static_cast<double>(i));
        REQUIRE(position->y == static_cast<double>(i * 10));
        REQUIRE(position->z == static_cast<double>(i * 20));
    }
}

TEST_CASE("SpawnConfig can initialize multiple components", "[simulation]") {
    conductor::Random random(12345);
	conductor::SimulationClock clock;
    conductor::World world(random);

    world.register_component<Position>();
    world.register_component<Velocity>();
    world.register_component<Battery>();

    conductor::SpawnConfig config;

    config.component<Position>(
        Position{1.0, 2.0}
    );

    config.component<Velocity>(
        Velocity{3.0, 4.0}
    );

    config.component<Battery>(
        Battery{100.0}
    );

    world.register_spawner("robot", config);
    world.spawn_entities(1, "robot");

    conductor::Entity entity = world.get_entity(0);

    auto position = world.get_component<Position>(entity);
    auto velocity = world.get_component<Velocity>(entity);
    auto battery = world.get_component<Battery>(entity);

    REQUIRE(position->x == 1.0);
    REQUIRE(position->y == 2.0);

    REQUIRE(velocity->x == 3.0);
    REQUIRE(velocity->y == 4.0);

    REQUIRE(battery->charge == 100.0);
}

TEST_CASE("SpawnConfig can be reused", "[simulation]") {
    conductor::Random random(12345);
	conductor::SimulationClock clock;
    conductor::World world(random);

    world.register_component<Position>();

    conductor::SpawnConfig config;

    config.component<Position>(
        [](const conductor::SpawnContext& context) {
            return Position{
                static_cast<double>(context.index),
                0.0,
                0.0
            };
        }
    );

    world.register_spawner("position", config);
    world.spawn_entities(5, "position");
    world.spawn_entities(5, "position");

    REQUIRE(world.entities().size() == 10);

    for(std::size_t i = 0; i < 5; ++i) {
        conductor::Entity entity = world.get_entity(i);

        auto position = world.get_component<Position>(entity);

        REQUIRE(position->x == static_cast<double>(i));
    }

    for(std::size_t i = 5; i < 10; ++i) {
        conductor::Entity entity = world.get_entity(i);

        auto position = world.get_component<Position>(entity);

        REQUIRE(
            position->x ==
            static_cast<double>(i - 5)
        );
    }
}

TEST_CASE("Spawning zero entities does nothing", "[simulation]") {
    conductor::Random random(12345);
	conductor::SimulationClock clock;
    conductor::World world(random);

    world.register_component<Position>();

    conductor::SpawnConfig config;

    config.component<Position>(
        Position{10.0, 20.0}
    );

    world.register_spawner("position", config);
    world.spawn_entities(0, "position");

    REQUIRE(world.entities().size() == 0);
}

TEST_CASE("SpawnConfig random initialization is deterministic", "[simulation]") {
    conductor::Random random_a(12345);
    conductor::Random random_b(12345);
    conductor::SimulationClock clock_a;
    conductor::SimulationClock clock_b;

    conductor::World world_a(random_a);
    conductor::World world_b(random_b);


    world_a.register_component<Position>();
    world_b.register_component<Position>();

    conductor::SpawnConfig config_a;
    conductor::SpawnConfig config_b;

    config_a.component<Position>(
        [](const conductor::SpawnContext& context) {
            return Position{
                context.random.next_double(),
                context.random.next_double(),
                context.random.next_double()
            };
        }
    );

    config_b.component<Position>(
        [](const conductor::SpawnContext& context) {
            return Position{
                context.random.next_double(),
                context.random.next_double(),
                context.random.next_double()
            };
        }
    );

    world_a.register_spawner("position", config_a);
    world_b.register_spawner("position", config_b);
    world_a.spawn_entities(100, "position");
    world_b.spawn_entities(100, "position");

    for(std::size_t i = 0; i < 100; ++i) {
        conductor::Entity a = world_a.get_entity(i);
        conductor::Entity b = world_b.get_entity(i);

        auto position_a = world_a.get_component<Position>(a);
        auto position_b = world_b.get_component<Position>(b);

        REQUIRE(position_a->x == position_b->x);
        REQUIRE(position_a->y == position_b->y);
        REQUIRE(position_a->z == position_b->z);
    }
}

TEST_CASE("SpawnConfig does not retain generated component state", "[simulation]") {
    conductor::Random random(12345);
	conductor::SimulationClock clock;
    conductor::World world(random);

    world.register_component<Position>();

    conductor::SpawnConfig config;

    config.component<Position>(
        [](const conductor::SpawnContext& context) {
            return Position{
                static_cast<double>(context.index),
                0.0,
                0.0
            };
        }
    );

    world.register_spawner("position", config);
    world.spawn_entities(3, "position");
    world.spawn_entities(3, "position");

    for(std::size_t i = 0; i < 6; ++i) {
        conductor::Entity entity = world.get_entity(i);

        auto position = world.get_component<Position>(entity);

        const double expected =
            static_cast<double>(i < 3 ? i : i - 3);

        REQUIRE(position->x == expected);
    }
}