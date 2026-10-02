#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <format>

#include "include/conductor.h"

/* Helpers */
namespace {
    constexpr double frequency = 60.0;

    std::string telemetry_file = "./engine_telemetry.csv";

    void record(std::string data) {
        std::ofstream file(telemetry_file, std::ios::app);

        if(file.is_open()) {
            file << data << std::endl;
            file.close();
        } else {
            std::cerr << "Could not write to file" << std::endl;
        }
    } 
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

    struct TargetVelocity {
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

namespace {
    struct Drone {
    public:
        void configure(conductor::SpawnConfig& spawner) {
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
            .component<TargetVelocity>(TargetVelocity{})
            .component<Battery>(Battery{})
            .component<SensorData>(SensorData{})
            .component<DroneStatus>(DroneStatus{});
        }
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
            ctx.query(
                [&](conductor::Entity& e, const Position& position, const Target& target, SensorData& sensor) {
                    const double dx = target.x - position.x;
                    const double dy = target.y - position.y;

                    sensor.distance_to_target = std::sqrt(
                        dx * dx + 
                        dy * dy
                    );
                }
            );
        }
    };

    struct SteeringSystem {
        using Query = conductor::Query<
            conductor::Read<Position>,
            conductor::Read<Target>,
            conductor::Read<SensorData>,
            conductor::Write<Velocity>
        >;

        const double stiffness = 2.0;
        const double damping = 1.5;
        const double max_speed = 3.0;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            ctx.query(
                [&](conductor::Entity& e, const Position& position, const Target& target, const SensorData& sensor, Velocity& velocity) {
                    const double dx = target.x - position.x;
                    const double dy = target.y - position.y;

                    const double ax =
                        dx * stiffness -
                        velocity.x * damping;

                    const double ay =
                        dy * stiffness -
                        velocity.y * damping;

                    velocity.x += ax * ctx.delta_seconds();
                    velocity.y += ay * ctx.delta_seconds();

                    const double speed = std::sqrt(
                        velocity.x * velocity.x +
                        velocity.y * velocity.y
                    );

                    if(speed > max_speed) {
                        const double scale = max_speed / speed;

                        velocity.x *= scale;
                        velocity.y *= scale;
                    }
                }
            );
        }
    };

    struct IntegrationSystem {
        using Query = conductor::Query<
            conductor::Write<Position>,
            conductor::Read<Velocity>,
            conductor::Write<Target>,
            conductor::Read<TargetVelocity>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            ctx.query(
                [&](conductor::Entity& e, Position& position, const Velocity& velocity, Target& target, const TargetVelocity& t_velocity) {
                    position.x += velocity.x * ctx.delta_seconds();
                    position.y += velocity.y * ctx.delta_seconds();
                    target.x += t_velocity.x * ctx.delta_seconds();
                    target.y += t_velocity.y * ctx.delta_seconds();
                }
            );
        }
    };

    struct TargetSystem {
        using Query = conductor::Query<
            conductor::Write<TargetVelocity>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            ctx.query([&](conductor::Entity& e, TargetVelocity& t_velocity) {
                t_velocity.x += 
                    (10.0 * ctx.rand().next_double() - 5.0) *
                    ctx.delta_seconds();
                t_velocity.y += 
                    (10.0 * ctx.rand().next_double() - 5.0) *
                    ctx.delta_seconds();
            });
        }
    };

    struct BatterySystem {
        using Query = conductor::Query<
            conductor::Read<Velocity>,
            conductor::Write<Battery>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            ctx.query([&](conductor::Entity& e, const Velocity& velocity, Battery& battery) {
                const double speed = std::sqrt(
                    velocity.x * velocity.x +
                    velocity.y * velocity.y
                );

                if(battery.level > 0.0) {
                    battery.level -= 0.5 * std::exp(speed) * ctx.delta_seconds();
                }

                if(battery.level < 0.0) {
                    battery.level = 0.0;
                }
            });
        }
    };

    struct BatteryMonitorSystem {
        using Query = conductor::Query<
            conductor::Read<Battery>,
            conductor::Write<DroneStatus>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            ctx.query([&](conductor::Entity& e, const Battery& battery, DroneStatus& drone_status) {
                if(battery.level <= 10.0) {
                    drone_status.state = DroneState::LowBattery;
                }
            });
        }
    };

    struct DroneRemovalSystem {
        using Query = conductor::Query<
            conductor::Read<DroneStatus>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            ctx.query([&](conductor::Entity& e, const DroneStatus& drone_status) {
                if(drone_status.state == DroneState::LowBattery) {
                    ctx.commands().destroy_entity(e);
                }
            });
        }
    };

    struct TelemetrySystem {
        using Query = conductor::Query<
            conductor::Read<Position>,
            conductor::Read<Velocity>,
            conductor::Read<Target>,
            conductor::Read<TargetVelocity>,
            conductor::Read<Battery>,
            conductor::Read<DroneStatus>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            ctx.query(
                [&](
                    conductor::Entity& e, 
                    const Position& position, 
                    const Velocity& velocity,
                    const Target& target,
                    const TargetVelocity& t_velocity,
                    const Battery& battery,
                    const DroneStatus& drone_status
                ) {
                    const std::string telemetry_line = 
                        std::format(
                            "{},{},{},{},{},{},{},{},{},{},{},{},{}",
                            ctx.clock().total_seconds(),
                            e.index(),
                            e.generation(),
                            position.x,
                            position.y,
                            velocity.x,
                            velocity.y,
                            target.x,
                            target.y,
                            t_velocity.x,
                            t_velocity.y,
                            battery.level,
                            (drone_status.state == DroneState::Active ? "active" : "low battery")
                        );

                    record(telemetry_line);
                }
            );
        }
    };
}

/* Plugin management */
namespace {
    class DroneSimPlugin final : public conductor::Plugin {
    public:
        std::string_view name() const override {
            return "drone_sim";
        }

        void configure(conductor::PluginConfig& config) override {
            config.component<Position>();
            config.component<Velocity>();
            config.component<Target>();
            config.component<TargetVelocity>();
            config.component<Battery>();
            config.component<SensorData>();
            config.component<DroneStatus>();

            config.entity_type<Drone>("drone");

            m_sensor = config.system(
                "sensor",
                frequency,
                SensorSystem{}
            );

            m_steering = config.system(
                "steering", 
                frequency,
                SteeringSystem{}
            );

            m_integration = config.system(
                "integration",
                frequency,
                IntegrationSystem{}
            );

            m_target = config.system(
                "target",
                frequency,
                TargetSystem{}
            );

            m_battery = config.system(
                "battery",
                frequency,
                BatterySystem{}
            );

            m_monitor = config.system(
                "monitor",
                frequency,
                BatteryMonitorSystem{}
            );

            m_removal = config.system(
                "removal",
                frequency,
                DroneRemovalSystem{}
            );

            m_steering.depends_on(m_integration);
        }

        void shutdown() override {}
    
    private:
        conductor::Scheduler::SystemHandle
            m_sensor, m_steering, m_integration,
            m_battery, m_monitor, m_removal,
            m_target;
    };
}

extern "C"
conductor::Plugin *conductor_create_plugin() {
    return new DroneSimPlugin();
}

extern "C"
void conductor_destroy_plugin(conductor::Plugin *plugin) {
    delete plugin;
}