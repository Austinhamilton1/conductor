#include <catch2/catch_test_macros.hpp>

#include "ecs/ComponentSet.h"

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

    struct Mass {
        float value;
    };

    struct Health {
        float value;
    };

    static_assert(conductor::Component<Position>);
    static_assert(conductor::Component<Velocity>);
    static_assert(conductor::Component<Mass>);
    static_assert(conductor::Component<Health>);
}

TEST_CASE("ComponentSet starts empty") {
    conductor::ComponentSet set;

    REQUIRE(set.empty());
    REQUIRE(set.size() == 0);

    REQUIRE_FALSE(
        set.contains<Position>()
    );

    REQUIRE_FALSE(
        set.contains<Velocity>()
    );
}

TEST_CASE("ComponentSet can add a component") {
    conductor::ComponentSet set;

    set.add<Position>();

    REQUIRE_FALSE(set.empty());
    REQUIRE(set.size() == 1);

    REQUIRE(set.contains<Position>());
    REQUIRE_FALSE(set.contains<Velocity>());
}

TEST_CASE("ComponentSet can contain multiple components") {
    conductor::ComponentSet set;

    set.add<Position>();
    set.add<Velocity>();
    set.add<Mass>();

    REQUIRE(set.size() == 3);

    REQUIRE(set.contains<Position>());
    REQUIRE(set.contains<Velocity>());
    REQUIRE(set.contains<Mass>());
    REQUIRE_FALSE(set.contains<Health>());
}

TEST_CASE("Adding the same component twice does not duplicate it") {
    conductor::ComponentSet set;

    set.add<Position>();
    set.add<Position>();

    REQUIRE(set.size() == 1);
    REQUIRE(set.contains<Position>());
}

TEST_CASE("ComponentSet can be cleared") {
    conductor::ComponentSet set;

    set.add<Position>();
    set.add<Velocity>();
    set.add<Mass>();

    REQUIRE(set.size() == 3);

    set.clear();

    REQUIRE(set.empty());
    REQUIRE(set.size() == 0);

    REQUIRE_FALSE(set.contains<Position>());
    REQUIRE_FALSE(set.contains<Velocity>());
    REQUIRE_FALSE(set.contains<Mass>());
}

TEST_CASE("ComponentSets with the same components are equal") {
    conductor::ComponentSet a;
    conductor::ComponentSet b;

    a.add<Position>();
    a.add<Velocity>();

    b.add<Position>();
    b.add<Velocity>();

    REQUIRE(a == b);
}

TEST_CASE("ComponentSet equality is independent of insertion order") {
    conductor::ComponentSet a;
    conductor::ComponentSet b;

    a.add<Position>();
    a.add<Velocity>();
    a.add<Mass>();

    b.add<Mass>();
    b.add<Position>();
    b.add<Velocity>();

    REQUIRE(a == b);
}

TEST_CASE("ComponentSets with different components are not equal") {
    conductor::ComponentSet a;
    conductor::ComponentSet b;

    a.add<Position>();
    a.add<Velocity>();

    b.add<Position>();
    b.add<Mass>();

    REQUIRE_FALSE(a == b);
}

TEST_CASE("ComponentSet can contain another ComponentSet") {
    conductor::ComponentSet archetype;
    conductor::ComponentSet query;

    archetype.add<Position>();
    archetype.add<Velocity>();
    archetype.add<Mass>();

    query.add<Position>();
    query.add<Velocity>();

    REQUIRE(archetype.contains(query));
}

TEST_CASE("ComponentSet does not contain a larger set") {
    conductor::ComponentSet archetype;
    conductor::ComponentSet query;

    archetype.add<Position>();
    archetype.add<Velocity>();

    query.add<Position>();
    query.add<Velocity>();
    query.add<Mass>();

    REQUIRE_FALSE(archetype.contains(query));
}

TEST_CASE("ComponentSet containment fails when a component is missing") {
    conductor::ComponentSet archetype;
    conductor::ComponentSet query;

    archetype.add<Position>();
    archetype.add<Mass>();

    query.add<Position>();
    query.add<Velocity>();

    REQUIRE_FALSE(archetype.contains(query));
}

TEST_CASE("Every ComponentSet contains an empty ComponentSet") {
    conductor::ComponentSet archetype;
    conductor::ComponentSet empty;

    archetype.add<Position>();
    archetype.add<Velocity>();

    REQUIRE(archetype.contains(empty));
}

TEST_CASE("Empty ComponentSet contains itself") {
    conductor::ComponentSet a;
    conductor::ComponentSet b;

    REQUIRE(a.contains(b));
}

TEST_CASE("ComponentSet contains itself") {
    conductor::ComponentSet set;

    set.add<Position>();
    set.add<Velocity>();
    set.add<Mass>();

    REQUIRE(set.contains(set));
}

TEST_CASE("ComponentSet can be copied") {
    conductor::ComponentSet original;

    original.add<Position>();
    original.add<Velocity>();
    original.add<Mass>();

    const conductor::ComponentSet copy = original;

    REQUIRE(copy == original);

    REQUIRE(copy.contains<Position>());
    REQUIRE(copy.contains<Velocity>());
    REQUIRE(copy.contains<Mass>());
}

TEST_CASE("Modifying a copied ComponentSet does not modify the original") {
    conductor::ComponentSet original;

    original.add<Position>();
    original.add<Velocity>();

    auto copy = original;

    copy.add<Mass>();

    REQUIRE(original.size() == 2);
    REQUIRE(copy.size() == 3);

    REQUIRE_FALSE(original.contains<Mass>());
    REQUIRE(copy.contains<Mass>());
}

TEST_CASE("ComponentSet contains empty set regardless of bitset size") {
    conductor::ComponentSet archetype;
    conductor::ComponentSet empty;

    archetype.add<Position>();
    archetype.add<Velocity>();

    REQUIRE(archetype.contains(empty));
    REQUIRE_FALSE(empty.contains(archetype));
}

TEST_CASE("ComponentSet containment handles different bitset sizes") {
    conductor::ComponentSet small;
    conductor::ComponentSet large;

    small.add<Position>();

    large.add<Position>();
    large.add<Velocity>();
    large.add<Mass>();

    REQUIRE(large.contains(small));
    REQUIRE_FALSE(small.contains(large));
}