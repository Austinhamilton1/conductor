#include <catch2/catch_test_macros.hpp>

#include "core/UUID.h"

TEST_CASE("UUIDs are unique", "[core]") {
    conductor::UUID a;
    conductor::UUID b;

    REQUIRE(a.value() != b.value());
}