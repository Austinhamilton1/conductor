#include "PluginManager.h"

#include <dlfcn.h>

#include <stdexcept>
#include <utility>

#include "Plugin.h"
#include "simulation/World.h"

namespace conductor {
    namespace {
        using CreatePluginFn = Plugin *(*)();
        using DestroyPluginFn = void (*)(Plugin *);

        template<typename T>
        T load_symbol(void *library, const char *name) {
            dlerror();

            void *sybmol = dlsym(library, name);

            const char *error = dlerror();

            if(error != nullptr) {
                throw std::runtime_error(
                    std::string("Failed to load plugin symbol '") +
                    name +
                    "': " +
                    error
                );
            }

            return reinterpret_cast<T>(sybmol);
        }
    }

    PluginManager::~PluginManager() {
        for(auto& [name, plugin] : m_plugins) {
            if(plugin.instance != nullptr && plugin.destroy != nullptr) {
                plugin.destroy(plugin.instance);
            }

            if(plugin.library != nullptr) {
                dlclose(plugin.library);
            }
        }
    }

    void PluginManager::load(const std::string& path, World& world) {
        void *library = dlopen(
            path.c_str(),
            RTLD_NOW | RTLD_LOCAL
        );

        if(library == nullptr) {
            throw std::runtime_error(
                std::string("Failed to load plugin '") +
                path +
                "': " +
                dlerror()
            );
        }

        Plugin *plugin = nullptr;
        DestroyPluginFn destroy = nullptr;

        try {
            const auto create =
                load_symbol<CreatePluginFn>(
                    library,
                    "conductor_create_plugin"
                );

            const auto destroy =
                load_symbol<DestroyPluginFn>(
                    library,
                    "conductor_destroy_plugin"
                );

            plugin = create();

            if(plugin == nullptr) {
                throw std::runtime_error(
                    "Plugin factory returned null"
                );
            }

            const std::string plugin_name(plugin->name());

            if(m_plugins.contains(plugin_name)) {
                destroy(plugin);
                throw std::runtime_error(
                    "Plugin already loaded: " + plugin_name
                );
            }

            plugin->initialize(world);

            m_plugins.emplace(
                plugin_name,
                LoadedPlugin{
                    library,
                    plugin,
                    destroy
                }
            );

            // Ownership has been transferred to m_plugins
            plugin = nullptr;
            library = nullptr;
        } catch(...) {
            if(plugin != nullptr && destroy != nullptr) {
                destroy(plugin);
            }

            if(library != nullptr) {
                dlclose(library);
            }
            
            throw;
        }
    }

    void PluginManager::shutdown(World& world) {
        for(auto& [name, plugin] : m_plugins) {
            if(plugin.instance != nullptr) {
                plugin.instance->shutdown(world);
            }
        }
    }

    bool PluginManager::loaded(std::string_view name) const {
        return m_plugins.contains(std::string(name));
    }
}