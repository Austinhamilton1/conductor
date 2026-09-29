#include "Simulation.h"

int main() {
    constexpr int num_drones = 10;
    constexpr double frequency = 10.0;
    
    conductor::ApplicationConfig config{
        .duration = 20.0,
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

    conductor::SpawnConfig spawner;
    spawner.component<Position>(
        [](const conductor::SpawnContext& ctx) {
            return Position{
                -10.0,
                2.0 * ctx.index
            };
        }
    )
    .component<Velocity>(Velocity{})
    .component<Target>(
        [](const conductor::SpawnContext& ctx) {
            return Target{
                10.0,
                2.0 * ctx.index
            };
        }
    )
    .component<Battery>(Battery{})
    .component<SensorData>(SensorData{})
    .component<DroneStatus>(DroneStatus{});

    app.world().register_spawner("drone", spawner);
    app.world().spawn_entities(num_drones, "drone");

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
    telemetry.depends_on(removal);

    app.world().finalize();

    app.run();
}