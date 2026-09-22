#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <string>

#include "core/Logger.h"

TEST_CASE("Log prints something", "[core]") {
    std::ostringstream out;

    conductor::Logger logger(conductor::LogLevel::Debug, out);

    logger.info("Hello");

    REQUIRE(out.str().find("Hello") != std::string::npos);
}