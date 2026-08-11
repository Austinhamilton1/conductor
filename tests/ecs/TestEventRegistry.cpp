#include <catch2/catch_test_macros.hpp>

#include "ecs/EntityRegistry.h"

TEST_CASE("Registry creates entities") {
    conductor::EntityRegistry registry;

    auto a = registry.create();
    auto b = registry.create();

    REQUIRE(a != b);
    REQUIRE(registry.alive(a));
    REQUIRE(registry.alive(b));
    REQUIRE(registry.size() == 2);
}

TEST_CASE("Registry destroys entities") {
    conductor::EntityRegistry registry;

    auto entity = registry.create();

    REQUIRE(registry.alive(entity));

    registry.destroy(entity);

    REQUIRE_FALSE(registry.alive(entity));
    REQUIRE(registry.size() == 0);
}

TEST_CASE("Destroyed entity handle remains invalid after slot reuse") {
    conductor::EntityRegistry registry;

    auto old_entity = registry.create();

    registry.destroy(old_entity);

    auto new_entity = registry.create();

    REQUIRE(old_entity.index() == new_entity.index());
    REQUIRE(old_entity.generation() != new_entity.generation());

    REQUIRE_FALSE(registry.alive(old_entity));
    REQUIRE(registry.alive(new_entity));
}

TEST_CASE("Destroying an already destroyed entity is harmless") {
    conductor::EntityRegistry registry;

    auto entity = registry.create();

    registry.destroy(entity);
    registry.destroy(entity);

    REQUIRE(registry.size() == 0);
}