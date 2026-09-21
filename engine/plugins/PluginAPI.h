#pragma once

#include "Plugin.h"

#if defined(_WIN32)
    #define CONDUCTOR_PLUGIN_EXPORT __declspec(dllexport)
#else
    #define CONDUCTOR_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif 

extern "C" {
    CONDUCTOR_PLUGIN_EXPORT conductor::Plugin *conductor_create_plugin();

    CONDUCTOR_PLUGIN_EXPORT void conductor_destroy_plugin(conductor::Plugin *plugin);
}