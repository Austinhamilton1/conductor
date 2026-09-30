#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "core/NonCopyable.h"

namespace conductor {
    class World;
    class Plugin;
    class EventBus;

    class PluginManager : public NonCopyable {
    public:
        PluginManager() = default;
        ~PluginManager();

        void load(const std::string& path, World& world, EventBus& events);
        void shutdown();
        
        bool loaded(std::string_view name) const;

    private:
        struct LoadedPlugin {
            void *library{nullptr};
            Plugin *instance{nullptr};

            void (*destroy)(Plugin *){nullptr};
        };

        std::unordered_map<std::string, LoadedPlugin> m_plugins;
    };
}