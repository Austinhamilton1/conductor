#include <catch2/catch_test_macros.hpp>

#include "Application.h"

class TestApplication : public conductor::Application {
public:
    bool initialized = false;
    bool updated = false;
    bool shutdownCalled = false;

protected:
    void initialize() override {
        initialized = true;
    }

    void update() override {
        updated = true;
        stop();
    }

    void shutdown() override {
        shutdownCalled = true;
    }
};

TEST_CASE("Application lifecycle") {
    TestApplication app;

    app.run();

    REQUIRE(app.initialized);
    REQUIRE(app.updated);
    REQUIRE(app.shutdownCalled);
}