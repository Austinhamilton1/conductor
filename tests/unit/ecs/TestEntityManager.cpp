#include <catch2/catch_test_macros.hpp>

#include <thread>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "ecs/EntityManager.h"

namespace {
    constexpr int entities_per_thread = 100;

    int get_thread_count() {
        const unsigned int reported =
            std::thread::hardware_concurrency();

        // Keep tests repeatable and avoid creating excessive threads.
        return static_cast<int>(
            std::min(reported == 0 ? 4u : reported, 8u)
        );
    }

    struct EntityHash {
        std::size_t operator()(const conductor::Entity& e) const noexcept {
            std::size_t h1 = std::hash<conductor::Entity::Index>{}(e.index());
            std::size_t h2 = std::hash<conductor::Entity::Generation>{}(e.generation());

            return h1 ^ (h2 << 1);
        }
    };
}

TEST_CASE("Registry creates entities", "[ecs]") {
    conductor::EntityManager registry;

    auto a = registry.create();
    auto b = registry.create();

    REQUIRE(a != b);
    REQUIRE(registry.alive(a));
    REQUIRE(registry.alive(b));
    REQUIRE(registry.size() == 2);
}

TEST_CASE("Registry destroys entities", "[ecs]") {
    conductor::EntityManager registry;

    auto entity = registry.create();

    REQUIRE(registry.alive(entity));

    registry.destroy(entity);

    REQUIRE_FALSE(registry.alive(entity));
    REQUIRE(registry.size() == 0);
}

TEST_CASE("Destroyed entity handle remains invalid after slot reuse", "[ecs]") {
    conductor::EntityManager registry;

    auto old_entity = registry.create();

    registry.destroy(old_entity);

    auto new_entity = registry.create();

    REQUIRE(old_entity.index() == new_entity.index());
    REQUIRE(old_entity.generation() != new_entity.generation());

    REQUIRE_FALSE(registry.alive(old_entity));
    REQUIRE(registry.alive(new_entity));
}

TEST_CASE("Destroying an already destroyed entity is harmless", "[ecs]") {
    conductor::EntityManager registry;

    auto entity = registry.create();

    registry.destroy(entity);
    registry.destroy(entity);

    REQUIRE(registry.size() == 0);
}


TEST_CASE("Multithreaded creation is synchronized", "[ecs]") {
    conductor::EntityManager registry;

    const int thread_count = get_thread_count();
    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    const int total_entities = thread_count * entities_per_thread;
    std::vector<conductor::Entity> entities(total_entities);

    for(int i = 0; i < thread_count; i++) {
        threads.emplace_back([&, i]() {
            for(int j = 0; j < entities_per_thread; j++) {
                auto e = registry.create();
                entities[i * entities_per_thread + j] = e;
            }
        });
    }

    for(auto& thread : threads) {
        thread.join();
    }

    for(auto e : entities) {
        REQUIRE(registry.alive(e));
    }

    REQUIRE(registry.size() == total_entities);
}

TEST_CASE("Multithreaded destruction is synchronized", "[ecs]") {
    conductor::EntityManager registry;

    const int thread_count = get_thread_count();
    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    const int total_entities = thread_count * entities_per_thread;
    std::vector<conductor::Entity> entities(total_entities);

    for(int i = 0; i < total_entities; i++) {
        entities[i] = registry.create();
    }

    for(int i = 0; i < thread_count; i++) {
        threads.emplace_back([&, i]() {
            for(int j = 0; j < entities_per_thread; j++) {
                registry.destroy(entities[i * entities_per_thread + j]);
            }
        });
    }

    for(auto& thread : threads) {
        thread.join();
    }

    for(auto e : entities) {
        REQUIRE_FALSE(registry.alive(e));
    }

    REQUIRE(registry.size() == 0);
}