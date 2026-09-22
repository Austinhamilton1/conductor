#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <limits>

#include "ecs/ComponentRegistry.h"

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

    struct Health {
        float value;
    };

    struct Acceleration {
        float x;
        float y;
        float z;
    };

    struct Mass {
        float value;
    };

    static_assert(conductor::Component<Position>);
    static_assert(conductor::Component<Velocity>);
    static_assert(conductor::Component<Health>);
    static_assert(conductor::Component<Acceleration>);
    static_assert(conductor::Component<Mass>);
}

TEST_CASE("Component types receive unique IDs", "[ecs]") {
    const auto position =
        conductor::ComponentRegistry::register_component<Position>();

    const auto velocity =
        conductor::ComponentRegistry::register_component<Velocity>();

    const auto health =
        conductor::ComponentRegistry::register_component<Health>();

    REQUIRE(position != velocity);
    REQUIRE(position != health);
    REQUIRE(velocity != health);
}

TEST_CASE("Registering the same component type returns the same ID", "[ecs]") {
    const auto first =
        conductor::ComponentRegistry::register_component<Position>();

    const auto second =
        conductor::ComponentRegistry::register_component<Position>();

    REQUIRE(first == second);
}

TEST_CASE("Component info contains correct metadata", "[ecs]") {
    const auto& info =
        conductor::ComponentRegistry::info<Position>();

    REQUIRE(
        info.id ==
        conductor::ComponentRegistry::register_component<Position>()
    );

    REQUIRE(info.size == sizeof(Position));
    REQUIRE(info.alignment == alignof(Position));
}

TEST_CASE("Component info can be retrieved by ID", "[ecs]") {
    const auto id =
        conductor::ComponentRegistry::register_component<Velocity>();

    const auto& typed_info =
        conductor::ComponentRegistry::info<Velocity>();

    const auto& id_info =
        conductor::ComponentRegistry::info(id);

    REQUIRE(id_info.id == typed_info.id);
    REQUIRE(id_info.size == typed_info.size);
    REQUIRE(id_info.alignment == typed_info.alignment);
}

TEST_CASE("Component registry reports registered component count", "[ecs]") {
    const auto initial_size =
        conductor::ComponentRegistry::size();

    conductor::ComponentRegistry::register_component<Acceleration>();
    conductor::ComponentRegistry::register_component<Mass>();

    REQUIRE(
        conductor::ComponentRegistry::size() ==
        initial_size + 2
    );
}

TEST_CASE("Component registry can reserve capacity", "[ecs]") {
    REQUIRE_FALSE(
        conductor::ComponentRegistry::frozen()
    );

    REQUIRE_NOTHROW(
        conductor::ComponentRegistry::reserve(128)
    );
}