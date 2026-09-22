#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <iostream>

#include "core/Random.h"
#include "ecs/Entity.h"
#include "ecs/Query.h"
#include "ecs/SystemContext.h"
#include "runtime/Application.h"

/* Initialization */
namespace {
    conductor::Random rand(12345);
    constexpr int num_drones = 10;
    constexpr double num_seconds = 10.0;
    constexpr double frequency = 10.0;
}

/* Components for drone simulation */
namespace {
    struct Position {
        double x = 0.0;
        double y = 0.0;
    };

    struct Velocity {
        double x = 0.0;
        double y = 0.0;
    };

    struct Target {
        double x = 0.0;
        double y = 0.0;
    };

    struct Battery {
        double level = 100.0;
    };

    struct SensorData {
        double distance_to_target = 0.0;
    };

    enum class DroneState {
        Active,
        LowBattery
    };

    struct DroneStatus {
        DroneState state = DroneState::Active;
    };
}

/* Systems for drone system */
namespace {
    struct SensorSystem {
        using Query = conductor::Query<
            conductor::Read<Position>,
            conductor::Read<Target>,
            conductor::Write<SensorData>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            for(auto [position, target, sensor] : ctx.query()) {
                const double dx = target.x - position.x;
                const double dy = target.y - position.y;

                sensor.distance_to_target = std::sqrt(
                    dx * dx + 
                    dy * dy
                );
            }
        }
    };

    struct SteeringSystem {
        using Query = conductor::Query<
            conductor::Read<Position>,
            conductor::Read<Target>,
            conductor::Read<SensorData>,
            conductor::Write<Velocity>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            for(auto [position, target, sensor, velocity] : ctx.query()) {
                const double dx = target.x - position.x;
                const double dy = target.y - position.y;

                const double speed = sensor.distance_to_target * rand.next_double() * 0.025;

                velocity.x += dx * speed * ctx.delta_seconds();
                velocity.y += dy * speed * ctx.delta_seconds();
            }
        }
    };

    struct IntegrationSystem {
        using Query = conductor::Query<
            conductor::Write<Position>,
            conductor::Read<Velocity>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            for(auto [position, velocity] : ctx.query()) {
                position.x += velocity.x * ctx.delta_seconds();
                position.y += velocity.y * ctx.delta_seconds();
            }
        }
    };

    struct BatterySystem {
        using Query = conductor::Query<
            conductor::Read<Velocity>,
            conductor::Write<Battery>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            for(auto [velocity, battery] : ctx.query()) {
                const double speed = std::sqrt(
                    velocity.x * velocity.x +
                    velocity.y * velocity.y
                );

                if(battery.level > 0.0) {
                    battery.level -= std::exp(speed) * ctx.delta_seconds();
                }
            }
        }
    };

    struct BatteryMonitorSystem {
        using Query = conductor::Query<
            conductor::Read<Battery>,
            conductor::Write<DroneStatus>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            for(auto [battery, drone_status] : ctx.query()) {
                if(battery.level <= 10.0) {
                    drone_status.state = DroneState::LowBattery;
                }
            }
        }
    };

    struct DroneRemovalSystem {
        using Query = conductor::Query<
            conductor::Read<DroneStatus>
        >;

        inline static int removals = 0;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            for(auto [drone_status] : ctx.query()) {
                if(drone_status.state == DroneState::LowBattery) {
                    // Need to think about this one
                }
            }
        }
    };

    struct TelemetrySystem {
        using Query = conductor::Query<
            conductor::Read<Position>,
            conductor::Read<Velocity>,
            conductor::Read<Battery>,
            conductor::Read<DroneStatus>
        >;

        inline static int updates = 0;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            for(auto [position, velocity, battery, drone_status] : ctx.query()) {
                // Need to think about this one
            }
        }
    };
}

/* Telemetry */
namespace {
    struct DroneSnapshot {
        Position pos;
        Velocity vel;
        Target tar;
        Battery battery;
        SensorData sensor;
        DroneStatus status;

        bool operator==(const DroneSnapshot& other) const {
            return (
                pos.x == other.pos.x &&
                pos.y == other.pos.y &&
                vel.x == other.vel.x &&
                vel.y == other.vel.y &&
                tar.x == other.tar.x &&
                tar.y == other.tar.y &&
                battery.level == other.battery.level &&
                sensor.distance_to_target == other.sensor.distance_to_target &&
                status.state == other.status.state
            );
        }
    };

    struct SimulationSnapshot {
        double time;

        std::vector<DroneSnapshot> drones;

        int destroyed_drones;
        int telemetry_updates;

        bool operator==(const SimulationSnapshot& other) const {
            for(int i = 0; i < drones.size(); i++) {
                if(drones[i] != other.drones[i])
                    return false;
            }

            return (
                time == other.time &&
                destroyed_drones == other.destroyed_drones &&
                telemetry_updates == other.telemetry_updates
            );
        }
    };

    SimulationSnapshot run_application() {
        conductor::Application app;

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

        auto& positions = app.world().storage().get<Position>();
        auto& velocities = app.world().storage().get<Velocity>();
        auto& targets = app.world().storage().get<Target>();
        auto& batteries = app.world().storage().get<Battery>();
        auto& sensors = app.world().storage().get<SensorData>();
        auto& statuses = app.world().storage().get<DroneStatus>();

        std::vector<DroneSnapshot> drones;
        for(int i = 0; i < num_drones; i++) {

            conductor::Entity e = app.world().get_entity(i);

            Position p = *positions.get(e);
            Velocity v = *velocities.get(e);
            Target t = *targets.get(e);
            Battery b = *batteries.get(e);
            SensorData s = *sensors.get(e);
            DroneStatus st = *statuses.get(e);

            drones.push_back(DroneSnapshot{
                .pos = p,
                .vel = v,
                .tar = t,
                .battery = b,
                .sensor = s,
                .status = st
            });
        }
        

        return SimulationSnapshot{
            .time = app.simulation_clock().total_seconds(),
            .drones = drones,
            .destroyed_drones = DroneRemovalSystem::removals,
            .telemetry_updates = TelemetrySystem::updates
        };
    }
}


TEST_CASE("Conductor engine is deterministic", "[integration]") {
    auto first_snapshot = run_application();
    auto second_snapshot = run_application();

    REQUIRE(first_snapshot == second_snapshot);
}