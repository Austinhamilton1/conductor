#pragma once

#include <string_view>

namespace conductor {
    class World;

    class Plugin {
    public:
        virtual ~Plugin() = default;

        virtual std::string_view name() const = 0;

        virtual void initialize(World& world) = 0;
        virtual void shutdown(World&) {}
    };
}