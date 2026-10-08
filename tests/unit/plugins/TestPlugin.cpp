#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

#include "core/Random.h"
#include "runtime/Application.h"
#include "TestComponents.h"

TEST_CASE("Plugin can be loaded", "[plugins]") {
    conductor::ApplicationConfig config{
        .duration = 10.0,
        .time_scale = 1.0,
        .step_size = 1.0 / 60.0,
        .seed = 12345
    };

    conductor::Application app(config);

    REQUIRE_FALSE(app.plugins().loaded("hello"));

    REQUIRE_NOTHROW(
        app.load_plugin(TEST_PLUGIN_PATH)
    );

    REQUIRE(app.plugins().loaded("hello"));
}

TEST_CASE("Duplicate plugin names are rejected", "[plugins]") {
    conductor::ApplicationConfig config{
        .duration = 10.0,
        .time_scale = 1.0,
        .step_size = 1.0 / 60.0,
        .seed = 12345
    };

    conductor::Application app(config);

    app.load_plugin(TEST_PLUGIN_PATH);

    REQUIRE_THROWS_AS(
        app.load_plugin(TEST_PLUGIN_PATH),
        std::runtime_error
    );

    REQUIRE(app.plugins().loaded("hello"));
}

TEST_CASE("Plugin can register components", "[plugins]") {
    conductor::ApplicationConfig config{
        .duration = 10.0,
        .time_scale = 1.0,
        .step_size = 1.0 / 60.0,
        .seed = 12345
    };

    conductor::Application app(config);

    app.load_plugin(TEST_PLUGIN_PATH);

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
    conductor::ApplicationConfig config{
        .duration = 10.0,
        .time_scale = 1.0,
        .step_size = 1.0 / 60.0,
        .seed = 12345
    };

    conductor::Random rand;
    conductor::SimulationClock clock;
    conductor::Application app(config);

    app.load_plugin(
        TEST_PLUGIN_PATH
    );

    const auto entity = app.world().create_entity();

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

    app.world().add_component(
        entity,
        position
    );

    app.world().add_component(
        entity,
        velocity
    );

    app.world().finalize();

    app.world().update(1.0, app.get_context());

    auto result = app.world().get_component<PluginPosition>(entity);

    // Plugin system runs at 60 Hz.
    // With a one-second simulation delta, the
    // system should move the position by velocity.
    REQUIRE(result->x == 10.0f);
    REQUIRE(result->y == 20.0f);
    REQUIRE(result->z == 30.0f);

    app.plugins().shutdown();
}