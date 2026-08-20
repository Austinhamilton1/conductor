#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <type_traits>
#include <vector>

#include "ecs/ComponentStorage.h"
#include "ecs/Query.h"


namespace {

    struct Position {
        float x;
        float y;
    };


    struct Velocity {
        float x;
        float y;
    };


    struct Health {
        int value;
    };


    conductor::Entity make_entity(
        uint32_t index,
        uint32_t generation = 0
    ) {
        return conductor::Entity(
            index,
            generation
        );
    }

}


// ============================================================
// Single component - Read
// ============================================================

TEST_CASE("Query reads a single component") {
    conductor::ComponentStorage<Position> storage;

    const auto entity1 = make_entity(0);
    const auto entity2 = make_entity(1);
    const auto entity3 = make_entity(2);

    storage.add(
        entity1,
        Position{1.0f, 2.0f}
    );

    storage.add(
        entity2,
        Position{3.0f, 4.0f}
    );

    storage.add(
        entity3,
        Position{5.0f, 6.0f}
    );

    auto query =
        conductor::make_query<
            conductor::Read<Position>
        >(
            storage.view()
        );

    size_t count = 0;

    for(auto [position] : query) {
        static_assert(
            std::is_same_v<
                decltype(position),
                const Position&
            >
        );

        REQUIRE(position.x ==
            static_cast<float>(count * 2 + 1));

        REQUIRE(position.y ==
            static_cast<float>(count * 2 + 2));

        ++count;
    }

    REQUIRE(count == 3);
}


// ============================================================
// Single component - Write
// ============================================================

TEST_CASE("Query writes a single component") {
    conductor::ComponentStorage<Position> storage;

    const auto entity1 = make_entity(0);
    const auto entity2 = make_entity(1);

    storage.add(
        entity1,
        Position{1.0f, 2.0f}
    );

    storage.add(
        entity2,
        Position{3.0f, 4.0f}
    );

    auto query =
        conductor::make_query<
            conductor::Write<Position>
        >(
            storage.view()
        );

    for(auto [position] : query) {
        static_assert(
            std::is_same_v<
                decltype(position),
                Position&
            >
        );

        position.x += 10.0f;
        position.y += 20.0f;
    }

    REQUIRE(storage.get(entity1)->x == 11.0f);
    REQUIRE(storage.get(entity1)->y == 22.0f);

    REQUIRE(storage.get(entity2)->x == 13.0f);
    REQUIRE(storage.get(entity2)->y == 24.0f);
}


// ============================================================
// Read + Write
// ============================================================

TEST_CASE("Query reads one component and writes another") {
    conductor::ComponentStorage<Position> positions;
    conductor::ComponentStorage<Velocity> velocities;

    const auto entity1 = make_entity(0);
    const auto entity2 = make_entity(1);

    positions.add(
        entity1,
        Position{10.0f, 20.0f}
    );

    positions.add(
        entity2,
        Position{30.0f, 40.0f}
    );

    velocities.add(
        entity1,
        Velocity{1.0f, 2.0f}
    );

    velocities.add(
        entity2,
        Velocity{3.0f, 4.0f}
    );

    auto query =
        conductor::make_query<
            conductor::Read<Velocity>,
            conductor::Write<Position>
        >(
            velocities.view(),
            positions.view()
        );

    size_t count = 0;

    for(auto [velocity, position] : query) {
        static_assert(
            std::is_same_v<
                decltype(velocity),
                const Velocity&
            >
        );

        static_assert(
            std::is_same_v<
                decltype(position),
                Position&
            >
        );

        position.x += velocity.x;
        position.y += velocity.y;

        ++count;
    }

    REQUIRE(count == 2);

    REQUIRE(
        positions.get(entity1)->x == 11.0f
    );

    REQUIRE(
        positions.get(entity1)->y == 22.0f
    );

    REQUIRE(
        positions.get(entity2)->x == 33.0f
    );

    REQUIRE(
        positions.get(entity2)->y == 44.0f
    );
}


// ============================================================
// Intersection
// ============================================================

TEST_CASE("Query returns only entities containing all components") {
    conductor::ComponentStorage<Position> positions;
    conductor::ComponentStorage<Velocity> velocities;

    const auto entity1 = make_entity(0);
    const auto entity2 = make_entity(1);
    const auto entity3 = make_entity(2);
    const auto entity4 = make_entity(3);

    // Position:
    // entity1
    // entity2
    // entity4

    positions.add(
        entity1,
        Position{1.0f, 1.0f}
    );

    positions.add(
        entity2,
        Position{2.0f, 2.0f}
    );

    positions.add(
        entity4,
        Position{4.0f, 4.0f}
    );

    // Velocity:
    // entity1
    // entity3
    // entity4

    velocities.add(
        entity1,
        Velocity{10.0f, 10.0f}
    );

    velocities.add(
        entity3,
        Velocity{30.0f, 30.0f}
    );

    velocities.add(
        entity4,
        Velocity{40.0f, 40.0f}
    );

    auto query =
        conductor::make_query<
            conductor::Read<Velocity>,
            conductor::Write<Position>
        >(
            velocities.view(),
            positions.view()
        );

    std::vector<conductor::Entity> entities;

    for(auto [velocity, position] : query) {
        entities.push_back(
            // The entity is obtained from the query iterator
            // in the dedicated test below, so just verify data here.
            make_entity(
                static_cast<uint32_t>(
                    position.x
                ) / 1
            )
        );

        position.x += velocity.x;
    }

    REQUIRE(entities.size() == 2);

    // Only entity1 and entity4 should have been modified.
    REQUIRE(
        positions.get(entity1)->x == 11.0f
    );

    REQUIRE(
        positions.get(entity2)->x == 2.0f
    );

    REQUIRE(
        positions.get(entity3) == nullptr
    );

    REQUIRE(
        positions.get(entity4)->x == 44.0f
    );
}


// ============================================================
// Driver view
// ============================================================

TEST_CASE("Query uses the first view as the driver",) {
    conductor::ComponentStorage<Position> positions;
    conductor::ComponentStorage<Velocity> velocities;

    const auto entity1 = make_entity(0);
    const auto entity2 = make_entity(1);
    const auto entity3 = make_entity(2);

    positions.add(
        entity1,
        Position{1.0f, 1.0f}
    );

    positions.add(
        entity2,
        Position{2.0f, 2.0f}
    );

    positions.add(
        entity3,
        Position{3.0f, 3.0f}
    );

    velocities.add(
        entity1,
        Velocity{10.0f, 10.0f}
    );

    velocities.add(
        entity3,
        Velocity{30.0f, 30.0f}
    );

    auto query =
        conductor::make_query<
            conductor::Read<Velocity>,
            conductor::Read<Position>
        >(
            velocities.view(),
            positions.view()
        );

    size_t count = 0;

    for(auto [velocity, position] : query) {
        REQUIRE(
            position.x == velocity.x / 10.0f
        );

        ++count;
    }

    REQUIRE(count == 2);
}


// ============================================================
// Empty query
// ============================================================

TEST_CASE("Query is empty when the driver view is empty") {
    conductor::ComponentStorage<Position> positions;

    auto query =
        conductor::make_query<
            conductor::Read<Position>
        >(
            positions.view()
        );

    REQUIRE(query.empty());

    REQUIRE(
        query.begin() ==
        query.end()
    );
}


// ============================================================
// Empty intersection
// ============================================================

TEST_CASE("Query is empty when no entity has all components") {
    conductor::ComponentStorage<Position> positions;
    conductor::ComponentStorage<Velocity> velocities;

    const auto entity1 = make_entity(0);
    const auto entity2 = make_entity(1);

    positions.add(
        entity1,
        Position{1.0f, 1.0f}
    );

    velocities.add(
        entity2,
        Velocity{2.0f, 2.0f}
    );

    auto query =
        conductor::make_query<
            conductor::Read<Velocity>,
            conductor::Read<Position>
        >(
            velocities.view(),
            positions.view()
        );

    REQUIRE(query.empty());

    REQUIRE(
        query.begin() ==
        query.end()
    );
}


// ============================================================
// Multiple chunks
// ============================================================

TEST_CASE("Query works across multiple component storage chunks") {
    conductor::ComponentStorage<Position> positions;
    conductor::ComponentStorage<Velocity> velocities;

    constexpr size_t count = 10000;

    for(size_t i = 0; i < count; ++i) {
        const auto entity =
            make_entity(
                static_cast<uint32_t>(i)
            );

        positions.add(
            entity,
            Position{
                static_cast<float>(i),
                static_cast<float>(i)
            }
        );

        velocities.add(
            entity,
            Velocity{
                1.0f,
                2.0f
            }
        );
    }

    REQUIRE(positions.chunk_count() > 1);
    REQUIRE(velocities.chunk_count() > 1);

    auto query =
        conductor::make_query<
            conductor::Read<Velocity>,
            conductor::Write<Position>
        >(
            velocities.view(),
            positions.view()
        );

    size_t visited = 0;

    for(auto [velocity, position] : query) {
        position.x += velocity.x;
        position.y += velocity.y;

        ++visited;
    }

    REQUIRE(visited == count);

    const auto first =
        make_entity(0);

    const auto last =
        make_entity(
            static_cast<uint32_t>(count - 1)
        );

    REQUIRE(
        positions.get(first)->x == 1.0f
    );

    REQUIRE(
        positions.get(first)->y == 2.0f
    );

    REQUIRE(
        positions.get(last)->x ==
            static_cast<float>(count)
    );

    REQUIRE(
        positions.get(last)->y ==
            static_cast<float>(count - 1) + 2.0f
    );
}


// ============================================================
// Query preserves entity identity
// ============================================================

TEST_CASE("Query iterator exposes the correct entity") {
    conductor::ComponentStorage<Position> positions;
    conductor::ComponentStorage<Velocity> velocities;

    const auto entity1 = make_entity(10);
    const auto entity2 = make_entity(20);

    positions.add(
        entity1,
        Position{1.0f, 1.0f}
    );

    positions.add(
        entity2,
        Position{2.0f, 2.0f}
    );

    velocities.add(
        entity1,
        Velocity{10.0f, 10.0f}
    );

    velocities.add(
        entity2,
        Velocity{20.0f, 20.0f}
    );

    auto query =
        conductor::make_query<
            conductor::Read<Velocity>,
            conductor::Read<Position>
        >(
            velocities.view(),
            positions.view()
        );

    auto iterator = query.begin();

    REQUIRE(iterator != query.end());

    const auto first_entity =
        iterator.entity();

    ++iterator;

    REQUIRE(iterator != query.end());

    const auto second_entity =
        iterator.entity();

    REQUIRE(
        first_entity != second_entity
    );

    REQUIRE(
        (
            first_entity == entity1 ||
            first_entity == entity2
        )
    );

    REQUIRE(
        (
            second_entity == entity1 ||
            second_entity == entity2
        )
    );
}


// ============================================================
// Read-only query from const storage
// ============================================================

TEST_CASE("Query can read from const component storage") {
    conductor::ComponentStorage<Position> storage;

    const auto entity = make_entity(0);

    storage.add(
        entity,
        Position{10.0f, 20.0f}
    );

    const auto& const_storage = storage;

    auto query =
        conductor::make_query<
            conductor::Read<Position>
        >(
            const_storage.view()
        );

    size_t count = 0;

    for(auto [position] : query) {
        static_assert(
            std::is_same_v<
                decltype(position),
                const Position&
            >
        );

        REQUIRE(position.x == 10.0f);
        REQUIRE(position.y == 20.0f);

        ++count;
    }

    REQUIRE(count == 1);
}