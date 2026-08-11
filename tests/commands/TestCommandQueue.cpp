#include <catch2/catch_test_macros.hpp>

#include "commands/CommandQueue.h"

TEST_CASE("CommandQueue creates deferred entities") {
    conductor::EntityRegistry registry;
    conductor::CommandQueue queue;

    auto deferred = queue.create_entity();

    REQUIRE(deferred.valid());
    REQUIRE(registry.size() == 0);

    queue.apply(registry);

    REQUIRE(registry.size() == 1);

    auto entity = queue.resolve(deferred);

    REQUIRE(entity.valid());
    REQUIRE(registry.alive(entity));
}

TEST_CASE("CommandQueue resolves multiple deferred entities") {
    conductor::EntityRegistry registry;
    conductor::CommandQueue queue;

    auto first = queue.create_entity();
    auto second = queue.create_entity();
    auto third = queue.create_entity();

    queue.apply(registry);

    const auto first_entity = queue.resolve(first);
    const auto second_entity = queue.resolve(second);
    const auto third_entity = queue.resolve(third);

    REQUIRE(first_entity != second_entity);
    REQUIRE(first_entity != third_entity);
    REQUIRE(second_entity != third_entity);

    REQUIRE(registry.alive(first_entity));
    REQUIRE(registry.alive(second_entity));
    REQUIRE(registry.alive(third_entity));
}

TEST_CASE("Deferred entity cannot be resolved before apply") {
    conductor::EntityRegistry registry;
    conductor::CommandQueue queue;

    auto deferred = queue.create_entity();

    REQUIRE_THROWS_AS(
        queue.resolve(deferred),
        std::out_of_range
    );
}

TEST_CASE("Applying command queue consumes commands") {
    conductor::EntityRegistry registry;
    conductor::CommandQueue queue;

    queue.create_entity();

    queue.apply(registry);

    REQUIRE(registry.size() == 1);

    queue.apply(registry);

    REQUIRE(registry.size() == 1);
}

TEST_CASE("Deferred entity remains resolvable after apply") {
    conductor::EntityRegistry registry;
    conductor::CommandQueue queue;

    auto deferred = queue.create_entity();

    queue.apply(registry);

    auto entity = queue.resolve(deferred);

    REQUIRE(registry.alive(entity));
}

TEST_CASE("Clearing command queue invalidates deferred entities") {
    conductor::EntityRegistry registry;
    conductor::CommandQueue queue;

    auto deferred = queue.create_entity();

    queue.apply(registry);

    REQUIRE(queue.resolve(deferred).valid());

    queue.clear();

    REQUIRE_THROWS_AS(
        queue.resolve(deferred),
        std::out_of_range
    );
}

TEST_CASE("Destroy command is deferred") {
    conductor::EntityRegistry registry;
    conductor::CommandQueue queue;

    auto entity = registry.create();

    queue.destroy_entity(entity);

    REQUIRE(registry.alive(entity));

    queue.apply(registry);

    REQUIRE_FALSE(registry.alive(entity));
}