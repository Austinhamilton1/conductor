#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

#include "core/Random.h"
#include "runtime/Application.h"
#include "TestComponents.h"

TEST_CASE("Plugin can be loaded", "[plugins]") {
    conductor::Application app;

    REQUIRE_FALSE(app.plugins().loaded("hello"));

    REQUIRE_NOTHROW(
        app.plugins().load(
            TEST_PLUGIN_PATH,
            app.world()
        )
    );

    REQUIRE(app.plugins().loaded("hello"));
}

TEST_CASE("Duplicate plugin names are rejected", "[plugins]") {
    conductor::Application app;

    app.plugins().load(
        TEST_PLUGIN_PATH,
        app.world()
    );

    REQUIRE_THROWS_AS(
        app.plugins().load(
            TEST_PLUGIN_PATH,
            app.world()
        ),
        std::runtime_error
    );

    REQUIRE(app.plugins().loaded("hello"));
}

TEST_CASE("Plugin can register components", "[plugins]") {
    conductor::Application app;

    app.plugins().load(
        TEST_PLUGIN_PATH,
        app.world()
    );

    const auto position =
        conductor::ComponentRegistry::info<PluginPosition>();

    const auto velocity =
        conductor::ComponentRegistry::info<PluginVelocity>();

    REQUIRE(position.size == sizeof(PluginPosition));
    REQUIRE(position.alignment == alignof(PluginPosition));

    REQUIRE(velocity.size == sizeof(PluginVelocity));
    REQUIRE(velocity.alignment == alignof(PluginVelocity));
}

TEST_CASE("Plugin system executes during simulation", "[plugins]") {
    conductor::Random rand;
    conductor::World world(rand);

    conductor::PluginManager plugins;

    plugins.load(
        TEST_PLUGIN_PATH,
        world
    );

    const auto entity = world.create_entity();

    PluginPosition position{
        0.0f,
        0.0f,
        0.0f
    };

    PluginVelocity velocity{
        10.0f,
        20.0f,
        30.0f
    };

    world.add_component(
        entity,
        position
    );

    world.add_component(
        entity,
        velocity
    );

    world.finalize();

    world.update(1.0);

    const auto& storage = world.storage().get<PluginPosition>();
    const auto result = storage.get(entity);

    // Plugin system runs at 60 Hz.
    // With a one-second simulation delta, the
    // system should move the position by velocity.
    REQUIRE(result->x == 10.0f);
    REQUIRE(result->y == 20.0f);
    REQUIRE(result->z == 30.0f);

    plugins.shutdown(world);
}