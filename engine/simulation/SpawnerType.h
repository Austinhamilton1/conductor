#pragma once

#include <concepts>

namespace conductor {
    class SpawnConfig;

    template<typename T>
    concept Spawner = requires(T instance, conductor::SpawnConfig& config) {
        { instance.configure(config) } -> std::same_as<void>;
    };
}