#include <catch2/catch_test_macros.hpp>

#include "ecs/ComponentRegistry.h"

namespace {
    struct Position {
        float x;
        float y;
        float z;
    };

    static_assert(conductor::Component<Position>);
}

TEST_CASE("ComponentRegistry is immutable after freeze") {
    REQUIRE_FALSE(conductor::ComponentRegistry::frozen());

    conductor::ComponentRegistry::freeze();

    REQUIRE_THROWS_AS(
        conductor::ComponentRegistry::register_component<Position>(),
        std::logic_error
    );

    REQUIRE_THROWS_AS(
        conductor::ComponentRegistry::reserve(128),
        std::logic_error
    );
}