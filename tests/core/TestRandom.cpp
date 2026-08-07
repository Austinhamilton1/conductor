#include <catch2/catch_test_macros.hpp>

#include "core/Random.h"

TEST_CASE("Random produces deterministic sequences") {
    conductor::Random a(1234);
    conductor::Random b(1234);

    REQUIRE(a.next_int(0, 100) == b.next_int(0, 100));
    REQUIRE(a.next_int(0, 100) == b.next_int(0, 100));
    REQUIRE(a.next_int(0, 100) == b.next_int(0, 100));

    REQUIRE(a.next_float() == b.next_float());
    REQUIRE(a.next_float() == b.next_float());
    REQUIRE(a.next_float() == b.next_float());

    REQUIRE(a.next_double() == b.next_double());
    REQUIRE(a.next_double() == b.next_double());
    REQUIRE(a.next_double() == b.next_double());
}

TEST_CASE("Random produces sufficient randomness") {
    conductor::Random a(6467);
    conductor::Random b(5678);

    int same = 0;
    for(int i  = 0; i < 100; i++) {
        int a_rand = a.next_int(0, 100);
        int b_rand = b.next_int(0, 100);
        if(a_rand == b_rand) same++;
    }

    REQUIRE(same != 100);

    same = 0;
    for(int i  = 0; i < 100; i++) {
        float a_rand = a.next_float();
        float b_rand = b.next_float();
        if(a_rand == b_rand) same++;
    }

    REQUIRE(same != 100);

    same = 0;
    for(int i  = 0; i < 100; i++) {
        double a_rand = a.next_double();
        double b_rand = b.next_double();
        if(a_rand == b_rand) same++;
    }

    REQUIRE(same != 100);
}

TEST_CASE("Chance of 0 is always false") {
    conductor::Random a(9473);

    REQUIRE(a.chance(0.0f) == false);
    REQUIRE(a.chance(0.0f) == false);
    REQUIRE(a.chance(0.0f) == false);
    REQUIRE(a.chance(0.0f) == false);
    REQUIRE(a.chance(0.0f) == false);
}

TEST_CASE("Change of 1 is always true") {
    conductor::Random a(2034);

    REQUIRE(a.chance(1.0f) == true);
    REQUIRE(a.chance(1.0f) == true);
    REQUIRE(a.chance(1.0f) == true);
    REQUIRE(a.chance(1.0f) == true);
    REQUIRE(a.chance(1.0f) == true);
}

TEST_CASE("Integer range stays within bounds") {
    conductor::Random a(2954);

    for(int i = 0; i < 100; i++) {
        int r = a.next_int(0, 100);
        REQUIRE(r >= 0);
        REQUIRE(r <= 100);
    }
}

TEST_CASE("Float range stays within bounds") {
    conductor::Random a(6458);

    for(int i = 0; i < 100; i++) {
        float r = a.next_float();
        REQUIRE(r >= 0.0f);
        REQUIRE(r <= 1.0f);
    }
}

TEST_CASE("Double range stays within bounds") {
    conductor::Random a(3548);

    for(int i = 0; i < 100; i++) {
        double r = a.next_double();
        REQUIRE(r >= 0.0);
        REQUIRE(r <= 1.0);
    }
}