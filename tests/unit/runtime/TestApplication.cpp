#include <catch2/catch_test_macros.hpp>

#include "runtime/Application.h"

class TestApplication : public conductor::Application {
public:
    bool initialized = false;
    bool updated = false;
    bool shutdown_called = false;
    int update_count = 0;

protected:
    void initialize() override {
        simulation_clock().set_time_scale(2.0);
        initialized = true;
    }

    void update(double) override {
        updated = true;
        update_count++;
        
        if(update_count >= 5)
            stop();
    }

    void shutdown() override {
        shutdown_called = true;
    }
};

TEST_CASE("Application initializes and shuts down", "[runtime]") {
    TestApplication app;

    app.run();

    REQUIRE(app.initialized);
    REQUIRE(app.shutdown_called);
}

TEST_CASE("Update is called", "[runtime]") {
    TestApplication app;

    app.run();

    REQUIRE(app.updated);
    REQUIRE(app.update_count == 5);
}

TEST_CASE("Clock advances", "[runtime]") {
    TestApplication app;

    double before = app.clock().total_seconds();

    app.run();

    double after = app.clock().total_seconds();

    REQUIRE(after > before);
}

TEST_CASE("Simulation clock advances", "[runtime]") {
    TestApplication app;

    double before = app.simulation_clock().total_seconds();

    app.run();

    double after = app.simulation_clock().total_seconds();

    REQUIRE(after > before);
}

TEST_CASE("Simulation time scale works", "[runtime]") {
    TestApplication app;

    app.run();

    double real = app.clock().total_seconds();
    double sim = app.simulation_clock().total_seconds();

    REQUIRE(sim > real);
}