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

    constexpr int components_per_thread = 1000;

    int get_thread_count() {
        const unsigned int reported =
            std::thread::hardware_concurrency();

        // Keep tests repeatable and avoid creating excessive threads.
        return static_cast<int>(
            std::min(reported == 0 ? 4u : reported, 8u)
        );
    }
}


// -----------------------------------------------------------------------------
// Basic storage
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage starts empty", "[ecs]") {
    Storage storage;

    REQUIRE(storage.empty());
    REQUIRE(storage.size() == 0);
    REQUIRE(storage.chunk_count() == 0);
}


TEST_CASE("ComponentStorage adds a component", "[ecs]") {
    Storage storage;

    conductor::Entity entity(0, 0);

    const Position position{
        .x = 10.0f,
        .y = 20.0f
    };

    storage.add(entity, position);
    storage.flush_additions();

    REQUIRE(storage.size() == 1);
    REQUIRE_FALSE(storage.empty());

    REQUIRE(storage.contains(entity));
}


TEST_CASE("ComponentStorage retrieves a component", "[ecs]") {
    Storage storage;

    conductor::Entity entity(0, 0);

    storage.add(entity, {
        .x = 10.0f,
        .y = 20.0f
    });
    storage.flush_additions();

    Position* position = storage.get(entity);

    REQUIRE(position != nullptr);
    REQUIRE(position->x == 10.0f);
    REQUIRE(position->y == 20.0f);
}


TEST_CASE("ComponentStorage returns nullptr for missing component", "[ecs]") {
    Storage storage;

    conductor::Entity entity(0, 0);

    REQUIRE(storage.get(entity) == nullptr);
    REQUIRE_FALSE(storage.contains(entity));
}


TEST_CASE("ComponentStorage component can be modified", "[ecs]") {
    Storage storage;

    conductor::Entity entity(0, 0);

    storage.add(entity, {
        .x = 10.0f,
        .y = 20.0f
    });
    storage.flush_additions();

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

TEST_CASE("ComponentStorage stores multiple components", "[ecs]") {
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

    storage.flush_additions();

    REQUIRE(storage.size() == 3);

    REQUIRE(storage.get(first)->x == 1.0f);
    REQUIRE(storage.get(first)->y == 2.0f);

    REQUIRE(storage.get(second)->x == 3.0f);
    REQUIRE(storage.get(second)->y == 4.0f);

    REQUIRE(storage.get(third)->x == 5.0f);
    REQUIRE(storage.get(third)->y == 6.0f);
}


TEST_CASE("ComponentStorage supports different entity indices", "[ecs]") {
    Storage storage;

    conductor::Entity first(10, 0);
    conductor::Entity second(100, 0);
    conductor::Entity third(1000, 0);

    storage.add(first, {.x = 1.0f, .y = 2.0f});
    storage.add(second, {.x = 3.0f, .y = 4.0f});
    storage.add(third, {.x = 5.0f, .y = 6.0f});
    storage.flush_additions();

    REQUIRE(storage.size() == 3);

    REQUIRE(storage.get(first)->x == 1.0f);
    REQUIRE(storage.get(second)->x == 3.0f);
    REQUIRE(storage.get(third)->x == 5.0f);
}


// -----------------------------------------------------------------------------
// Duplicate components
// -----------------------------------------------------------------------------

TEST_CASE("Adding an existing component returns existing component", "[ecs]") {
    Storage storage;

    conductor::Entity entity(0, 0);

    storage.add(entity, {
        .x = 10.0f,
        .y = 20.0f
    });

    storage.add(entity, {
        .x = 30.0f,
        .y = 40.0f
    });

    storage.flush_additions();

    REQUIRE(storage.size() == 1);
}


// -----------------------------------------------------------------------------
// Generation handling
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage rejects stale entity generation", "[ecs]") {
    Storage storage;

    conductor::Entity old_entity(0, 0);
    conductor::Entity new_entity(0, 1);

    storage.add(old_entity, {
        .x = 10.0f,
        .y = 20.0f
    });
    storage.flush_additions();

    REQUIRE(storage.contains(old_entity));

    REQUIRE_FALSE(storage.contains(new_entity));
    REQUIRE(storage.get(new_entity) == nullptr);
}


TEST_CASE("ComponentStorage allows reused entity index with new generation", "[ecs]") {
    Storage storage;

    conductor::Entity old_entity(0, 0);
    conductor::Entity new_entity(0, 1);

    storage.add(old_entity, {
        .x = 10.0f,
        .y = 20.0f
    });
    storage.flush_additions();

    storage.remove(old_entity);
    storage.flush_removals();

    storage.add(new_entity, {
        .x = 30.0f,
        .y = 40.0f
    });
    storage.flush_additions();

    REQUIRE_FALSE(storage.contains(old_entity));
    REQUIRE(storage.contains(new_entity));

    REQUIRE(storage.get(new_entity)->x == 30.0f);
    REQUIRE(storage.get(new_entity)->y == 40.0f);
}


// -----------------------------------------------------------------------------
// Deferred removal
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage removal is deferred", "[ecs]") {
    Storage storage;

    conductor::Entity entity(0, 0);

    storage.add(entity, {
        .x = 10.0f,
        .y = 20.0f
    });
    storage.flush_additions();

    storage.remove(entity);

    REQUIRE(storage.size() == 1);
    REQUIRE(storage.contains(entity));
    REQUIRE(storage.get(entity) != nullptr);
}


TEST_CASE("ComponentStorage flush removes pending components", "[ecs]") {
    Storage storage;

    conductor::Entity entity(0, 0);

    storage.add(entity, {
        .x = 10.0f,
        .y = 20.0f
    });
    storage.flush_additions();

    storage.remove(entity);

    storage.flush_removals();

    REQUIRE(storage.empty());
    REQUIRE(storage.size() == 0);
    REQUIRE_FALSE(storage.contains(entity));
    REQUIRE(storage.get(entity) == nullptr);
}


TEST_CASE("ComponentStorage can flush removals multiple times", "[ecs]") {
    Storage storage;

    conductor::Entity first(0, 0);
    conductor::Entity second(1, 0);

    storage.add(first, {.x = 1.0f, .y = 2.0f});
    storage.add(second, {.x = 3.0f, .y = 4.0f});
    storage.flush_additions();

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

TEST_CASE("ComponentStorage preserves remaining components after removal", "[ecs]") {
    Storage storage;

    conductor::Entity first(0, 0);
    conductor::Entity second(1, 0);
    conductor::Entity third(2, 0);

    storage.add(first, {.x = 1.0f, .y = 2.0f});
    storage.add(second, {.x = 3.0f, .y = 4.0f});
    storage.add(third, {.x = 5.0f, .y = 6.0f});
    storage.flush_additions();

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


TEST_CASE("ComponentStorage updates moved entity location after removal", "[ecs]") {
    Storage storage;

    conductor::Entity first(0, 0);
    conductor::Entity second(1, 0);
    conductor::Entity third(2, 0);

    storage.add(first, {.x = 1.0f, .y = 2.0f});
    storage.add(second, {.x = 3.0f, .y = 4.0f});
    storage.add(third, {.x = 5.0f, .y = 6.0f});
    storage.flush_additions();

    storage.remove(first);
    storage.flush_removals();

    REQUIRE(storage.size() == 2);

    REQUIRE(storage.get(second)->x == 3.0f);
    REQUIRE(storage.get(second)->y == 4.0f);

    REQUIRE(storage.get(third)->x == 5.0f);
    REQUIRE(storage.get(third)->y == 6.0f);
}


TEST_CASE("ComponentStorage can remove last component", "[ecs]") {
    Storage storage;

    conductor::Entity first(0, 0);
    conductor::Entity second(1, 0);

    storage.add(first, {.x = 1.0f, .y = 2.0f});
    storage.add(second, {.x = 3.0f, .y = 4.0f});
    storage.flush_additions();

    storage.remove(second);
    storage.flush_removals();

    REQUIRE(storage.size() == 1);

    REQUIRE(storage.contains(first));
    REQUIRE_FALSE(storage.contains(second));

    REQUIRE(storage.get(first)->x == 1.0f);
    REQUIRE(storage.get(first)->y == 2.0f);
}


TEST_CASE("ComponentStorage removes the only component", "[ecs]") {
    Storage storage;

    conductor::Entity entity(0, 0);

    storage.add(entity, {.x = 1.0f, .y = 2.0f});
    storage.flush_additions();

    REQUIRE(storage.chunk_count() == 1);

    storage.remove(entity);
    storage.flush_removals();

    REQUIRE(storage.empty());
    REQUIRE(storage.chunk_count() == 0);
}


// -----------------------------------------------------------------------------
// Chunk behavior
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage allocates a new chunk when full", "[ecs]") {
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

    storage.flush_additions();
    REQUIRE(storage.size() == 4096);
    REQUIRE(storage.chunk_count() == 1);

    storage.add(
        conductor::Entity(4096, 0),
        {
            .x = 4.0f,
            .y = 4.0f
        }
    );

    storage.flush_additions();
    REQUIRE(storage.size() == 4097);
    REQUIRE(storage.chunk_count() == 2);
}


TEST_CASE("ComponentStorage stores components across multiple chunks", "[ecs]") {
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

    storage.flush_additions();
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

TEST_CASE("ComponentStorage iterates over all components", "[ecs]") {
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

    storage.flush_additions();

    size_t count = 0;

    for (Position& position : storage) {
        REQUIRE(position.x == static_cast<float>(count));
        REQUIRE(position.y == static_cast<float>(count * 2));

        ++count;
    }

    REQUIRE(count == 10);
}


TEST_CASE("ComponentStorage iteration remains dense after removal", "[ecs]") {
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

    storage.flush_additions();

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


TEST_CASE("ComponentStorage supports const iteration", "[ecs]") {
    Storage storage;

    storage.add(
        conductor::Entity(0, 0),
        {
            .x = 10.0f,
            .y = 20.0f
        }
    );

    storage.flush_additions();

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

TEST_CASE("ComponentStorage data provides dense access", "[ecs]") {
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

    storage.flush_additions();

    for (size_t i = 0; i < storage.size(); ++i) {
        REQUIRE(storage.data(i)->x == static_cast<float>(i));
    }
}


TEST_CASE("ComponentStorage remains densely indexed after swap removal", "[ecs]") {
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
    storage.flush_additions();

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

TEST_CASE("ComponentStorage reserve accepts entity capacity", "[ecs]") {
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

    storage.flush_additions();
    REQUIRE(storage.size() == 100);
}


// -----------------------------------------------------------------------------
// Invalid entities
// -----------------------------------------------------------------------------

TEST_CASE("ComponentStorage rejects invalid entity", "[ecs]") {
    Storage storage;

    const conductor::Entity invalid;

    REQUIRE_FALSE(storage.contains(invalid));
    REQUIRE(storage.get(invalid) == nullptr);
}


TEST_CASE("ComponentStorage remove ignores invalid entity", "[ecs]") {
    Storage storage;

    const conductor::Entity invalid;

    REQUIRE_NOTHROW(storage.remove(invalid));

    storage.flush_removals();

    REQUIRE(storage.empty());
}


// -----------------------------------------------------------------------------
// Multiple component storage types
// -----------------------------------------------------------------------------

TEST_CASE("Different component types have independent storage", "[ecs]") {
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

    positions.flush_additions();
    velocities.flush_additions();

    REQUIRE(positions.size() == 1);
    REQUIRE(velocities.size() == 1);

    REQUIRE(positions.get(entity)->x == 10.0f);
    REQUIRE(velocities.get(entity)->x == 1.0f);
}

TEST_CASE("ComponentStorage iterates across chunk boundaries", "[ecs]") {
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

    storage.flush_additions();

    REQUIRE(storage.chunk_count() == 2);

    size_t count = 0;

    for (const Position& position : storage) {
        REQUIRE(position.x == static_cast<float>(count));
        ++count;
    }

    REQUIRE(count == 5000);
}

TEST_CASE("ComponentStorage empty storage has valid iterators", "[ecs]") {
    Storage storage;

    REQUIRE(storage.begin() == storage.end());
}

TEST_CASE("ComponentStorage single component iterates correctly", "[ecs]") {
    Storage storage;

    storage.add(
        conductor::Entity(42, 0),
        {
            .x = 10.0f,
            .y = 20.0f
        }
    );

    storage.flush_additions();

    auto it = storage.begin();

    REQUIRE(it != storage.end());
    REQUIRE(it->x == 10.0f);
    REQUIRE(it->y == 20.0f);

    ++it;

    REQUIRE(it == storage.end());
}

TEST_CASE("Multithreaded delete queues in removal queue", "[ecs]") {
    Storage storage;

    int num_threads = get_thread_count();
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    int total_components = num_threads * components_per_thread;
    for(int i = 0; i < total_components; i++) {
        storage.add(
            conductor::Entity(i, 0),
            {
                .x = static_cast<float>(i),
                .y = static_cast<float>(i * 2)
            }
        );
    }

    storage.flush_additions();

    REQUIRE(storage.size() == total_components);

    for(int i = 0; i < num_threads; i++) {
        threads.emplace_back([&, i]() {
            for(int j = 0; j < components_per_thread; j++) {
                int entity_idx = i * components_per_thread + j;
                storage.remove(conductor::Entity(entity_idx, 0));
            }
        });
    }

    for(auto& thread : threads) {
        thread.join();
    }

    storage.flush_removals();

    REQUIRE(storage.size() == 0);
}