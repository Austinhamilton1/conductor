#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>

#include "ecs/ComponentStorage.h"
#include "ecs/Entity.h"


namespace {

    struct Position {
        float x;
        float y;

        bool operator==(const Position&) const = default;
    };


    struct Velocity {
        float x;
        float y;

        bool operator==(const Velocity&) const = default;
    };


    using Storage = conductor::ComponentStorage<Position>;

}


// -----------------------------------------------------------------------------
// Basic storage
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage starts empty") {
    Storage storage;

    REQUIRE(storage.empty());
    REQUIRE(storage.size() == 0);
    REQUIRE(storage.chunk_count() == 0);
}


TEST_CASE("ComponentStorage adds a component") {
    Storage storage;

    conductor::Entity entity(0, 0);

    const Position position{
        .x = 10.0f,
        .y = 20.0f
    };

    Position& stored = storage.add(entity, position);

    REQUIRE(storage.size() == 1);
    REQUIRE_FALSE(storage.empty());

    REQUIRE(stored == position);
    REQUIRE(storage.contains(entity));
}


TEST_CASE("ComponentStorage retrieves a component") {
    Storage storage;

    conductor::Entity entity(0, 0);

    storage.add(entity, {
        .x = 10.0f,
        .y = 20.0f
    });

    Position* position = storage.get(entity);

    REQUIRE(position != nullptr);
    REQUIRE(position->x == 10.0f);
    REQUIRE(position->y == 20.0f);
}


TEST_CASE("ComponentStorage returns nullptr for missing component") {
    Storage storage;

    conductor::Entity entity(0, 0);

    REQUIRE(storage.get(entity) == nullptr);
    REQUIRE_FALSE(storage.contains(entity));
}


TEST_CASE("ComponentStorage component can be modified") {
    Storage storage;

    conductor::Entity entity(0, 0);

    storage.add(entity, {
        .x = 10.0f,
        .y = 20.0f
    });

    Position* position = storage.get(entity);

    REQUIRE(position != nullptr);

    position->x = 50.0f;
    position->y = 75.0f;

    REQUIRE(storage.get(entity)->x == 50.0f);
    REQUIRE(storage.get(entity)->y == 75.0f);
}


// -----------------------------------------------------------------------------
// Multiple entities
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage stores multiple components") {
    Storage storage;

    conductor::Entity first(0, 0);
    conductor::Entity second(1, 0);
    conductor::Entity third(2, 0);

    storage.add(first, {
        .x = 1.0f,
        .y = 2.0f
    });

    storage.add(second, {
        .x = 3.0f,
        .y = 4.0f
    });

    storage.add(third, {
        .x = 5.0f,
        .y = 6.0f
    });

    REQUIRE(storage.size() == 3);

    REQUIRE(storage.get(first)->x == 1.0f);
    REQUIRE(storage.get(first)->y == 2.0f);

    REQUIRE(storage.get(second)->x == 3.0f);
    REQUIRE(storage.get(second)->y == 4.0f);

    REQUIRE(storage.get(third)->x == 5.0f);
    REQUIRE(storage.get(third)->y == 6.0f);
}


TEST_CASE("ComponentStorage supports different entity indices") {
    Storage storage;

    conductor::Entity first(10, 0);
    conductor::Entity second(100, 0);
    conductor::Entity third(1000, 0);

    storage.add(first, {.x = 1.0f, .y = 2.0f});
    storage.add(second, {.x = 3.0f, .y = 4.0f});
    storage.add(third, {.x = 5.0f, .y = 6.0f});

    REQUIRE(storage.size() == 3);

    REQUIRE(storage.get(first)->x == 1.0f);
    REQUIRE(storage.get(second)->x == 3.0f);
    REQUIRE(storage.get(third)->x == 5.0f);
}


// -----------------------------------------------------------------------------
// Duplicate components
// -----------------------------------------------------------------------------

TEST_CASE("Adding an existing component returns existing component") {
    Storage storage;

    conductor::Entity entity(0, 0);

    Position& first = storage.add(entity, {
        .x = 10.0f,
        .y = 20.0f
    });

    Position& second = storage.add(entity, {
        .x = 30.0f,
        .y = 40.0f
    });

    REQUIRE(&first == &second);

    REQUIRE(storage.size() == 1);

    REQUIRE(second.x == 10.0f);
    REQUIRE(second.y == 20.0f);
}


// -----------------------------------------------------------------------------
// Generation handling
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage rejects stale entity generation") {
    Storage storage;

    conductor::Entity old_entity(0, 0);
    conductor::Entity new_entity(0, 1);

    storage.add(old_entity, {
        .x = 10.0f,
        .y = 20.0f
    });

    REQUIRE(storage.contains(old_entity));

    REQUIRE_FALSE(storage.contains(new_entity));
    REQUIRE(storage.get(new_entity) == nullptr);
}


TEST_CASE("ComponentStorage allows reused entity index with new generation") {
    Storage storage;

    conductor::Entity old_entity(0, 0);
    conductor::Entity new_entity(0, 1);

    storage.add(old_entity, {
        .x = 10.0f,
        .y = 20.0f
    });

    storage.remove(old_entity);
    storage.flush_removals();

    storage.add(new_entity, {
        .x = 30.0f,
        .y = 40.0f
    });

    REQUIRE_FALSE(storage.contains(old_entity));
    REQUIRE(storage.contains(new_entity));

    REQUIRE(storage.get(new_entity)->x == 30.0f);
    REQUIRE(storage.get(new_entity)->y == 40.0f);
}


// -----------------------------------------------------------------------------
// Deferred removal
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage removal is deferred") {
    Storage storage;

    conductor::Entity entity(0, 0);

    storage.add(entity, {
        .x = 10.0f,
        .y = 20.0f
    });

    storage.remove(entity);

    REQUIRE(storage.size() == 1);
    REQUIRE(storage.contains(entity));
    REQUIRE(storage.get(entity) != nullptr);
}


TEST_CASE("ComponentStorage flush removes pending components") {
    Storage storage;

    conductor::Entity entity(0, 0);

    storage.add(entity, {
        .x = 10.0f,
        .y = 20.0f
    });

    storage.remove(entity);

    storage.flush_removals();

    REQUIRE(storage.empty());
    REQUIRE(storage.size() == 0);
    REQUIRE_FALSE(storage.contains(entity));
    REQUIRE(storage.get(entity) == nullptr);
}


TEST_CASE("ComponentStorage can flush removals multiple times") {
    Storage storage;

    conductor::Entity first(0, 0);
    conductor::Entity second(1, 0);

    storage.add(first, {.x = 1.0f, .y = 2.0f});
    storage.add(second, {.x = 3.0f, .y = 4.0f});

    storage.remove(first);
    storage.flush_removals();

    REQUIRE(storage.size() == 1);

    storage.remove(second);
    storage.flush_removals();

    REQUIRE(storage.empty());
}


// -----------------------------------------------------------------------------
// Swap removal
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage preserves remaining components after removal") {
    Storage storage;

    conductor::Entity first(0, 0);
    conductor::Entity second(1, 0);
    conductor::Entity third(2, 0);

    storage.add(first, {.x = 1.0f, .y = 2.0f});
    storage.add(second, {.x = 3.0f, .y = 4.0f});
    storage.add(third, {.x = 5.0f, .y = 6.0f});

    storage.remove(second);
    storage.flush_removals();

    REQUIRE(storage.size() == 2);

    REQUIRE(storage.contains(first));
    REQUIRE(storage.contains(third));

    REQUIRE(storage.get(first)->x == 1.0f);
    REQUIRE(storage.get(first)->y == 2.0f);

    REQUIRE(storage.get(third)->x == 5.0f);
    REQUIRE(storage.get(third)->y == 6.0f);
}


TEST_CASE("ComponentStorage updates moved entity location after removal") {
    Storage storage;

    conductor::Entity first(0, 0);
    conductor::Entity second(1, 0);
    conductor::Entity third(2, 0);

    storage.add(first, {.x = 1.0f, .y = 2.0f});
    storage.add(second, {.x = 3.0f, .y = 4.0f});
    storage.add(third, {.x = 5.0f, .y = 6.0f});

    storage.remove(first);
    storage.flush_removals();

    REQUIRE(storage.size() == 2);

    REQUIRE(storage.get(second)->x == 3.0f);
    REQUIRE(storage.get(second)->y == 4.0f);

    REQUIRE(storage.get(third)->x == 5.0f);
    REQUIRE(storage.get(third)->y == 6.0f);
}


TEST_CASE("ComponentStorage can remove last component") {
    Storage storage;

    conductor::Entity first(0, 0);
    conductor::Entity second(1, 0);

    storage.add(first, {.x = 1.0f, .y = 2.0f});
    storage.add(second, {.x = 3.0f, .y = 4.0f});

    storage.remove(second);
    storage.flush_removals();

    REQUIRE(storage.size() == 1);

    REQUIRE(storage.contains(first));
    REQUIRE_FALSE(storage.contains(second));

    REQUIRE(storage.get(first)->x == 1.0f);
    REQUIRE(storage.get(first)->y == 2.0f);
}


TEST_CASE("ComponentStorage removes the only component") {
    Storage storage;

    conductor::Entity entity(0, 0);

    storage.add(entity, {.x = 1.0f, .y = 2.0f});

    REQUIRE(storage.chunk_count() == 1);

    storage.remove(entity);
    storage.flush_removals();

    REQUIRE(storage.empty());
    REQUIRE(storage.chunk_count() == 0);
}


// -----------------------------------------------------------------------------
// Chunk behavior
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage allocates a new chunk when full") {
    Storage storage;

    for (uint32_t i = 0; i < 4096; ++i) {
        storage.add(
            conductor::Entity(i, 0),
            {
                .x = static_cast<float>(i),
                .y = static_cast<float>(i)
            }
        );
    }

    REQUIRE(storage.size() == 4096);
    REQUIRE(storage.chunk_count() == 1);

    storage.add(
        conductor::Entity(4096, 0),
        {
            .x = 4.0f,
            .y = 4.0f
        }
    );

    REQUIRE(storage.size() == 4097);
    REQUIRE(storage.chunk_count() == 2);
}


TEST_CASE("ComponentStorage stores components across multiple chunks") {
    Storage storage;

    constexpr uint32_t count = 5000;

    for (uint32_t i = 0; i < count; ++i) {
        storage.add(
            conductor::Entity(i, 0),
            {
                .x = static_cast<float>(i),
                .y = static_cast<float>(i * 10)
            }
        );
    }

    REQUIRE(storage.size() == count);
    REQUIRE(storage.chunk_count() == 2);

    for (uint32_t i = 0; i < count; ++i) {
        const Position* position =
            storage.get(conductor::Entity(i, 0));

        REQUIRE(position != nullptr);
        REQUIRE(position->x == static_cast<float>(i));
        REQUIRE(position->y == static_cast<float>(i * 10));
    }
}


// -----------------------------------------------------------------------------
// Iteration
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage iterates over all components") {
    Storage storage;

    for (uint32_t i = 0; i < 10; ++i) {
        storage.add(
            conductor::Entity(i, 0),
            {
                .x = static_cast<float>(i),
                .y = static_cast<float>(i * 2)
            }
        );
    }

    size_t count = 0;

    for (Position& position : storage) {
        REQUIRE(position.x == static_cast<float>(count));
        REQUIRE(position.y == static_cast<float>(count * 2));

        ++count;
    }

    REQUIRE(count == 10);
}


TEST_CASE("ComponentStorage iteration remains dense after removal") {
    Storage storage;

    for (uint32_t i = 0; i < 6; ++i) {
        storage.add(
            conductor::Entity(i, 0),
            {
                .x = static_cast<float>(i),
                .y = 0.0f
            }
        );
    }

    storage.remove(conductor::Entity(2, 0));
    storage.remove(conductor::Entity(4, 0));

    storage.flush_removals();

    REQUIRE(storage.size() == 4);

    size_t count = 0;

    for (const Position& position : storage) {
        REQUIRE(position.x != 2.0f);
        REQUIRE(position.x != 4.0f);

        ++count;
    }

    REQUIRE(count == 4);
}


TEST_CASE("ComponentStorage supports const iteration") {
    Storage storage;

    storage.add(
        conductor::Entity(0, 0),
        {
            .x = 10.0f,
            .y = 20.0f
        }
    );

    const Storage& const_storage = storage;

    size_t count = 0;

    for (const Position& position : const_storage) {
        REQUIRE(position.x == 10.0f);
        REQUIRE(position.y == 20.0f);

        ++count;
    }

    REQUIRE(count == 1);
}


// -----------------------------------------------------------------------------
// Dense indexing
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage data provides dense access") {
    Storage storage;

    for (uint32_t i = 0; i < 10; ++i) {
        storage.add(
            conductor::Entity(i, 0),
            {
                .x = static_cast<float>(i),
                .y = 0.0f
            }
        );
    }

    for (size_t i = 0; i < storage.size(); ++i) {
        REQUIRE(storage.data(i)->x == static_cast<float>(i));
    }
}


TEST_CASE("ComponentStorage remains densely indexed after swap removal") {
    Storage storage;

    for (uint32_t i = 0; i < 5; ++i) {
        storage.add(
            conductor::Entity(i, 0),
            {
                .x = static_cast<float>(i),
                .y = 0.0f
            }
        );
    }

    storage.remove(conductor::Entity(1, 0));
    storage.flush_removals();

    REQUIRE(storage.size() == 4);

    /*
     * Entity 4 should have been moved into the slot
     * previously occupied by entity 1.
     */
    REQUIRE(storage.get(conductor::Entity(4, 0))->x == 4.0f);

    /*
     * The dense array should contain exactly four
     * components and no holes.
     */
    size_t count = 0;

    for (const Position& position : storage) {
        REQUIRE(position.x != 1.0f);
        ++count;
    }

    REQUIRE(count == 4);
}


// -----------------------------------------------------------------------------
// Reserve
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage reserve accepts entity capacity") {
    Storage storage;

    storage.reserve(1000);

    for (uint32_t i = 0; i < 100; ++i) {
        storage.add(
            conductor::Entity(i, 0),
            {
                .x = static_cast<float>(i),
                .y = 0.0f
            }
        );
    }

    REQUIRE(storage.size() == 100);
}


// -----------------------------------------------------------------------------
// Invalid entities
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage rejects invalid entity") {
    Storage storage;

    const conductor::Entity invalid;

    REQUIRE_FALSE(storage.contains(invalid));
    REQUIRE(storage.get(invalid) == nullptr);
}


TEST_CASE("ComponentStorage remove ignores invalid entity") {
    Storage storage;

    const conductor::Entity invalid;

    REQUIRE_NOTHROW(storage.remove(invalid));

    storage.flush_removals();

    REQUIRE(storage.empty());
}


// -----------------------------------------------------------------------------
// Multiple component storage types
// -----------------------------------------------------------------------------

TEST_CASE("Different component types have independent storage") {
    conductor::ComponentStorage<Position> positions;
    conductor::ComponentStorage<Velocity> velocities;

    conductor::Entity entity(0, 0);

    positions.add(entity, {
        .x = 10.0f,
        .y = 20.0f
    });

    velocities.add(entity, {
        .x = 1.0f,
        .y = 2.0f
    });

    REQUIRE(positions.size() == 1);
    REQUIRE(velocities.size() == 1);

    REQUIRE(positions.get(entity)->x == 10.0f);
    REQUIRE(velocities.get(entity)->x == 1.0f);
}

TEST_CASE("ComponentStorage iterates across chunk boundaries") {
    Storage storage;

    for (uint32_t i = 0; i < 5000; ++i) {
        storage.add(
            conductor::Entity(i, 0),
            {
                .x = static_cast<float>(i),
                .y = 0.0f
            }
        );
    }

    REQUIRE(storage.chunk_count() == 2);

    size_t count = 0;

    for (const Position& position : storage) {
        REQUIRE(position.x == static_cast<float>(count));
        ++count;
    }

    REQUIRE(count == 5000);
}

TEST_CASE("ComponentStorage empty storage has valid iterators") {
    Storage storage;

    REQUIRE(storage.begin() == storage.end());
}

TEST_CASE("ComponentStorage single component iterates correctly") {
    Storage storage;

    storage.add(
        conductor::Entity(42, 0),
        {
            .x = 10.0f,
            .y = 20.0f
        }
    );

    auto it = storage.begin();

    REQUIRE(it != storage.end());
    REQUIRE(it->x == 10.0f);
    REQUIRE(it->y == 20.0f);

    ++it;

    REQUIRE(it == storage.end());
}