#include <catch2/catch_test_macros.hpp>

#include "runtime/Application.h"

class TestApplication : public conductor::Application {
public:
    bool initialized = false;
    bool updated = false;
    bool shutdown_called = false;
    int update_count = 0;

    TestApplication(conductor::ApplicationConfig config) : Application(config) {}

protected:
    void initialize() override {
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
    conductor::ApplicationConfig config{
        .duration = 10.0,
        .time_scale = 2.0,
        .step_size = 1.0 / 60.0,
        .seed = 12345
    };

    TestApplication app(config);

    app.run();

    REQUIRE(app.initialized);
    REQUIRE(app.shutdown_called);
}

TEST_CASE("Update is called", "[runtime]") {
    conductor::ApplicationConfig config{
        .duration = 10.0,
        .time_scale = 2.0,
        .step_size = 1.0 / 60.0,
        .seed = 12345
    };

    TestApplication app(config);

    app.run();

    REQUIRE(app.updated);
    REQUIRE(app.update_count == 5);
}

TEST_CASE("Clock advances", "[runtime]") {
    conductor::ApplicationConfig config{
        .duration = 10.0,
        .time_scale = 2.0,
        .step_size = 1.0 / 60.0,
        .seed = 12345
    };

    TestApplication app(config);

    double before = app.clock().total_seconds();

    app.run();

    double after = app.clock().total_seconds();

    REQUIRE(after > before);
}

TEST_CASE("Simulation clock advances", "[runtime]") {
    conductor::ApplicationConfig config{
        .duration = 10.0,
        .time_scale = 2.0,
        .step_size = 1.0 / 60.0,
        .seed = 12345
    };

    TestApplication app(config);

    double before = app.simulation_clock().total_seconds();

    app.run();

    double after = app.simulation_clock().total_seconds();

    REQUIRE(after > before);
}

TEST_CASE("Simulation time scale works", "[runtime]") {
    conductor::ApplicationConfig config{
        .duration = 10.0,
        .time_scale = 2.0,
        .step_size = 1.0 / 60.0,
        .seed = 12345
    };

    TestApplication app(config);

    app.run();

    double real = app.clock().total_seconds();
    double sim = app.simulation_clock().total_seconds();

    REQUIRE(sim > real);
}