#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "ecs/ComponentStorage.h"
#include "ecs/ComponentView.h"

namespace {
    struct Position {
        float x;
        float y;
        float z;
    };

    using Storage = conductor::ComponentStorage<Position>;
}

TEST_CASE("Empty storage returns empty view") {
    Storage storage;

    auto view = storage.view();

    REQUIRE(view.empty());
    REQUIRE(view.size() == 0);
    REQUIRE(view.begin() == view.end());
}

TEST_CASE("Adding components changes view") {
    Storage storage;

    conductor::Entity e0(0, 0);
    conductor::Entity e1(1, 0);
    conductor::Entity e2(2, 0);

    storage.add(e0, Position{1, 2, 3});
    storage.add(e1, Position{4, 5, 6});
    storage.add(e2, Position{7, 8, 9});

    auto view = storage.view();

    REQUIRE(view.size() == 3);

    size_t count = 0;

    for(auto& position : view) {
        ++count;
    }

    REQUIRE(count == 3);
}

TEST_CASE("Mutating through view mutates underlying component") {
    Storage storage;

    conductor::Entity e0(0, 0);
    conductor::Entity e1(1, 0);

    storage.add(e0, Position{1, 2, 3});
    storage.add(e1, Position{4, 5, 6});

    auto view = storage.view();

    for(auto& position : view) {
        position.x += 10.0f;
    }

    REQUIRE_THAT(storage.get(e0)->x, Catch::Matchers::WithinAbs(11.0f, 0.0001));
    REQUIRE_THAT(storage.get(e1)->x, Catch::Matchers::WithinAbs(14.0f, 0.0001));
}

TEST_CASE("Const view is const") {
    Storage storage;

    const auto& const_storage = storage;

    auto view = const_storage.view();

    REQUIRE(std::is_same_v<decltype(*view.begin()), const Position&>);
}

TEST_CASE("Views work across chunk boundaries") {
    conductor::ComponentStorage<Position> storage;

    for(uint32_t i = 0; i < 5000; i++) {
        storage.add(conductor::Entity(i, 0), Position{
            static_cast<float>(i),
            static_cast<float>(i * 2),
            static_cast<float>(i * 3)
        });
    }

    auto view = storage.view();

    REQUIRE(view.size() == 5000);

    for(auto& position : view) {
        position.x = 10.0f;
    }

    for(size_t i = 0; i < 5000; i++) {
        REQUIRE_THAT(storage.data(i)->x, Catch::Matchers::WithinAbs(10.0f, 0.0001));
    }
}

TEST_CASE("ComponentView can lookup components by entity") {
    Storage storage;

    conductor::Entity entity(0, 0);

    storage.add(entity, Position{1.0f, 2.0f, 3.0f});

    auto view = storage.view();

    REQUIRE(view.contains(entity));

    Position* position = view.get(entity);

    REQUIRE(position != nullptr);
    REQUIRE(position->x == 1.0f);
    REQUIRE(position->y == 2.0f);
    REQUIRE(position->z == 3.0f);
}

TEST_CASE("ComponentView returns null for missing entity") {
    Storage storage;

    conductor::Entity entity(0, 0);

    auto view = storage.view();

    REQUIRE_FALSE(view.contains(entity));
    REQUIRE(view.get(entity) == nullptr);
}