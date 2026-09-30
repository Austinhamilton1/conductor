#pragma once

#include <string_view>

#include "PluginConfig.h"

namespace conductor {
    class Plugin {
    public:
        virtual ~Plugin() = default;

        virtual std::string_view name() const = 0;

        virtual void configure(PluginConfig& config) = 0;
        virtual void shutdown() {}
    };
}