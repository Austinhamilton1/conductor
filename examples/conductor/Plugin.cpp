#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <format>

#include "ecs/Entity.h"
#include "ecs/Query.h"
#include "ecs/SystemContext.h"
#include "runtime/Application.h"
#include "plugins/Plugin.h"
#include "plugins/PluginAPI.h"

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

        const double stiffness = 2.0;
        const double damping = 1.5;
        const double max_speed = 3.0;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            for(auto [position, target, sensor, velocity] : ctx.query()) {
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
                    battery.level -= 0.5 * std::exp(speed) * ctx.delta_seconds();
                }

                if(battery.level < 0.0) {
                    battery.level = 0.0;
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

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            for(auto it = ctx.query().begin(); it != ctx.query().end(); it++) {
                conductor::Entity e = it.entity();
                auto [drone_status] = *it;

                if(drone_status.state == DroneState::LowBattery) {
                    ctx.commands().destroy_entity(e);
                }
            }
        }
    };

    struct TelemetrySystem {
        using Query = conductor::Query<
            conductor::Read<Position>,
            conductor::Read<Velocity>,
            conductor::Read<Target>,
            conductor::Read<Battery>,
            conductor::Read<DroneStatus>
        >;

        inline static int updates = 0;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT>& ctx) {
            for(auto it = ctx.query().begin(); it != ctx.query().end(); it++) {
                conductor::Entity e = it.entity();
                auto [position, velocity, target, battery, drone_status] = *it;

                const std::string telemetry_line = 
                    std::format(
                        "{},({}; {}),({}; {}),({}; {}),({}; {}),{},{}",
                        ctx.clock().total_seconds(),
                        e.index(),
                        e.generation(),
                        position.x,
                        position.y,
                        velocity.x,
                        velocity.y,
                        target.x,
                        target.y,
                        battery.level,
                        (drone_status.state == DroneState::Active ? "active" : "low battery")
                    );

                record(telemetry_line);
            }
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

        void initialize(conductor::World& world) override {
            world.register_component<Position>();
            world.register_component<Velocity>();
            world.register_component<Target>();
            world.register_component<Battery>();
            world.register_component<SensorData>();
            world.register_component<DroneStatus>();

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

            world.register_spawner("drone", spawner);

            m_sensor = world.add_system(
                "sensor",
                frequency,
                SensorSystem{}
            );

            m_steering = world.add_system(
                "steering", 
                frequency,
                SteeringSystem{}
            );

            m_integration = world.add_system(
                "integration",
                frequency,
                IntegrationSystem{}
            );

            m_battery = world.add_system(
                "battery",
                frequency,
                BatterySystem{}
            );

            m_monitor = world.add_system(
                "monitor",
                frequency,
                BatteryMonitorSystem{}
            );

            m_removal = world.add_system(
                "removal",
                frequency,
                DroneRemovalSystem{}
            );

            m_telemetry = world.add_system(
                "telemetry",
                frequency,
                TelemetrySystem{}
            );

            m_steering.depends_on(m_integration);
            m_telemetry.depends_on(m_removal);
        }

        void shutdown(conductor::World&) override {}
    
    private:
        conductor::Scheduler::SystemHandle
            m_sensor, m_steering, m_integration,
            m_battery, m_monitor, m_removal,
            m_telemetry;
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