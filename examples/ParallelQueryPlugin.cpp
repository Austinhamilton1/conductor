#include "include/conductor.h"

namespace {
    struct Position {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };
}

namespace {
    struct IntegrationSystem {
        using Query = conductor::Query<
            conductor::Write<Position>
        >;

        template<typename BoundQueryT>
        void update(conductor::SystemContext<BoundQueryT> ctx) {
            ctx.parallel_query([&](conductor::Entity& e, Position& pos) {
                double x = pos.x;
                double y = pos.y;
                double z = pos.z;

                for(int i = 0; i < 100; ++i) {
                    x = std::sin(x) + std::cos(y);
                    y = std::sin(y) + std::cos(z);
                    z = std::sin(z) + std::cos(x);
                }

                pos.x = x;
                pos.y = y;
                pos.z = z;
            });
        }
    };
}

namespace {
    struct Spawner {
        void configure(conductor::SpawnConfig& config) {
            config
                .component<Position>(
                    [](const conductor::SpawnContext& ctx) {
                        return Position{
                            static_cast<double>(ctx.index),
                            static_cast<double>(ctx.index),
                            static_cast<double>(ctx.index)
                        };
                    }
                );
        }
    };
};

class ParallelQueryPlugin : public conductor::Plugin {
    std::string_view name() const override {
        return "parallel_query";
    }

    void configure(conductor::PluginConfig& config) override {
        config.component<Position>();

        config.entity_type<Spawner>("object");

        m_integration = config.system(
            "integration",
            60.0,
            IntegrationSystem{}
        );
    }

    conductor::Scheduler::SystemHandle m_integration;
};

extern "C"
conductor::Plugin *conductor_create_plugin() {
    return new ParallelQueryPlugin();
}

extern "C"
void conductor_destroy_plugin(conductor::Plugin *plugin) {
    delete plugin;
}