#pragma once

#include <string>
#include <vector>

#include "runtime/ApplicationConfig.h"

struct EntityConifg {
    std::string type;
    std::size_t count;
};

struct ConductorConfig {
    conductor::ApplicationConfig app_config;
    std::vector<std::string> plugins;
    std::vector<EntityConifg> entities;
    int success = 0;
};

ConductorConfig load_conductor_config(const std::string& filename);