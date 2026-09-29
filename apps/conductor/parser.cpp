#include "parser.h"

#include <iostream>
#include <filesystem>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

std::string make_path_relative_to_parent(
    const std::string& filepath_str,
    const std::string& target_path
) {
    fs::path yaml_dir = fs::absolute(filepath_str).parent_path();

    fs::path full_path = yaml_dir / target_path;

    return fs::weakly_canonical(full_path).string();
}

ConductorConfig load_conductor_config(const std::string& filename) {
    fs::path filepath = filename;

    fs::path dir_path = filepath.parent_path();

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
                std::string path = plugin.as<std::string>();
                config.plugins.push_back(
                    make_path_relative_to_parent(filename, path)
                );
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