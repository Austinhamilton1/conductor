#include "plugins/PluginAPI.h"
#include "../TestComponents.h"
#include "simulation/World.h"
#include "ecs/Query.h"
#include "scheduler/Scheduler.h"
#include "ecs/Entity.h"

namespace {
    class PluginMovementSystem {
    public:
        using Query = conductor::Query<
            conductor::Read<PluginVelocity>,
            conductor::Write<PluginPosition>
        >;

        inline static int executions = 0;

        template<typename BoundQuery>
        void update(conductor::SystemContext<BoundQuery>& ctx) {
            ++executions;

            ctx.query(
                [&](conductor::Entity& e, const PluginVelocity& velocity, PluginPosition& position) {
                    position.x += velocity.x * 
                        ctx.delta_seconds();

                    position.y += velocity.y *
                        ctx.delta_seconds();

                    position.z += velocity.z *
                        ctx.delta_seconds();
                }
            );
        }

        static void reset() {
            executions = 0;
        }
    };

    class MovementPlugin final : public conductor::Plugin {
    public:
        std::string_view name() const override {
            return "hello";
        }

        void configure(conductor::PluginConfig& config) override {
            config.component<PluginPosition>();
            config.component<PluginVelocity>();

            handle = config.system<PluginMovementSystem>(
                "plugin_movement",
                60.0,
                PluginMovementSystem{}
            );
        }

        void shutdown() override {}
    
    private:
        conductor::Scheduler::SystemHandle handle;
    };
}

extern "C"
conductor::Plugin *conductor_create_plugin() {
    return new MovementPlugin();
}

extern "C"
void conductor_destroy_plugin(conductor::Plugin *plugin) {
    delete plugin;
}