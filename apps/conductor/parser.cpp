#include "parser.h"

#include <iostream>
#include <yaml-cpp/yaml.h>

ConductorConfig load_conductor_config(const std::string& filename) {
    try {
        ConductorConfig config;

        YAML::Node root = YAML::LoadFile(filename);

        config.app_config.duration = 
            root["simulation"]["duration"].as<double>();
        
        config.app_config.time_scale =
            root["simulation"]["time_scale"].as<double>();

        config.app_config.step_size =
            root["simulation"]["step_size"].as<double>();

        config.app_config.seed =
            root["simulation"]["seed"].as<uint64_t>();

        if(root["plugins"] && root["plugins"].IsSequence()) {
            for(const auto& plugin : root["plugins"]) {
                config.plugins.push_back(plugin.as<std::string>());
            }
        }

        if(root["entities"] && root["entities"].IsSequence()) {
            for(const auto& entity : root["entities"]) {
                config.entities.push_back(EntityConifg{
                    .type = entity["type"].as<std::string>(),
                    .count = entity["count"].as<std::size_t>()
                });
            }
        }

        config.success = 1;

        return config;
    } catch(const YAML::Exception& e) {
        std::cerr << "Error parsing YAML: " << e.what() << std::endl;
        return ConductorConfig{};
    }
}