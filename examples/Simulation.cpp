#include "Simulation.h"

int main() {
    conductor::ApplicationConfig config{
        .duration = 10.0,
        .time_scale = 10000.0,
        .step_size = 1.0 / 60.0,
        .seed = 12345
    };

    RecordApp app(config);

    app.world().register_component<Position>();
    app.world().register_component<Velocity>();
    app.world().register_component<Target>();
    app.world().register_component<Battery>();
    app.world().register_component<SensorData>();
    app.world().register_component<DroneStatus>();

    for(int i = 0; i < num_drones; i++) {
        conductor::Entity e = app.world().create_entity();

        Position p{-10.0, 2.0 * i};
        Velocity v{};
        Target t{10.0, 2.0 * i};
        Battery b{};
        SensorData s{};
        DroneStatus st{};

        app.world().add_component(e, p);
        app.world().add_component(e, v);
        app.world().add_component(e, t);
        app.world().add_component(e, b),
        app.world().add_component(e, s);
        app.world().add_component(e, st);
    }

    auto sensor = app.world().add_system(
        "sensor",
        frequency,
        SensorSystem{}
    );

    auto steering = app.world().add_system(
        "steering", 
        frequency,
        SteeringSystem{}
    );

    auto integration = app.world().add_system(
        "integration",
        frequency,
        IntegrationSystem{}
    );

    auto battery = app.world().add_system(
        "battery",
        frequency,
        BatterySystem{}
    );

    auto monitor = app.world().add_system(
        "monitor",
        frequency,
        BatteryMonitorSystem{}
    );

    auto removal = app.world().add_system(
        "removal",
        frequency,
        DroneRemovalSystem{}
    );

    auto telemetry = app.world().add_system(
        "telemetry",
        frequency,
        TelemetrySystem{}
    );

    steering.depends_on(integration);

    app.world().finalize();

    app.run();
}