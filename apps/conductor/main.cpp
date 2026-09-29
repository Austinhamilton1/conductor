#include <iostream>

#include "parser.h"

#include "runtime/Application.h"
#include "runtime/ApplicationConfig.h"

namespace {
    class OutputApplication : public conductor::Application {
        void initialize() {
            std::cout << "Starting simulation..." << std::flush;
        }

        void shutdown() {
            std::cout << "Done." << std::endl;
        }

    public:
        OutputApplication(conductor::ApplicationConfig config) 
            : Application(config) {}
    };
}

int main(int argc, char **argv) {
    if(argc != 2) {
        std::cerr << "Error: Usage - conductor <simulation config>\n";
        return 1;
    }

    ConductorConfig config = load_conductor_config(argv[1]);
    if(!config.success) {
        std::cerr << "Error: Could not load config file\n";
        return 1;
    }

    std::cout << "Conductor Configuration:\n";
    std::cout << "--------------------------------------------\n";
    std::cout << "Duration: " << config.app_config.duration << "s\n";
    std::cout << "Time Scale: " << config.app_config.time_scale << "x\n";
    std::cout << "Step Size: " << config.app_config.step_size << "s\n";
    std::cout << "Random Seed: " << config.app_config.seed << std::endl;
    std::cout << "--------------------------------------------\n";
    std::cout << "Plugins:\n";
    std::cout << "--------------------------------------------\n";

    for(const auto& plugin : config.plugins) {
        std::cout << plugin << std::endl;
    }

    std::cout << "--------------------------------------------\n";
    std::cout << "Entities:\n";
    std::cout << "--------------------------------------------\n";
    
    for(const auto& entity : config.entities) {
        std::cout << "Type: " << entity.type << std::endl;
        std::cout << "Count: " << entity.count << std::endl;
    }

    std::cout << "--------------------------------------------\n";

    OutputApplication app(config.app_config);
    for(const auto& plugin : config.plugins) {
        app.load_plugin(plugin);
    }

    for(const auto& entity : config.entities) {
        app.world().spawn_entities(entity.count, entity.type);
    }

    app.run();

    return 0;
}