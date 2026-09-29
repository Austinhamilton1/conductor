#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

#include "World.h"
#include "core/Random.h"
#include "ecs/Entity.h"
#include "ecs/ComponentType.h"

namespace conductor {
    struct SpawnContext {
        Entity entity;
        std::size_t index;
        Random& random;
    };

    class SpawnConfig {
    public:
        template<Component T>
        SpawnConfig& component(T value) {
            m_initializers.emplace_back(
                [value = std::move(value)](
                    World& world,
                    Entity entity,
                    SpawnContext&
                ) mutable {
                    world.add_component<T>(entity, value);
                }
            );

            return *this;
        }

        template<Component T, typename F>
        requires std::invocable<F&, const SpawnContext&> &&
                 std::convertible_to<
                    std::invoke_result_t<F&, const SpawnContext&>,
                    T
                 >
        SpawnConfig& component(F initializer) {
            m_initializers.emplace_back(
                [initializer = std::move(initializer)](
                    World& world,
                    Entity entity,
                    SpawnContext& context
                ) mutable {
                    T value = initializer(context);
                    world.add_component<T>(entity, value);
                }
            );

            return *this;
        }

        void initialize(World& world, Entity entity, SpawnContext& context) const {
            for(auto& initializer : m_initializers) {
                initializer(world, entity, context);
            }
        }

    private:
        std::vector<std::function<void(World&, Entity, SpawnContext&)>> m_initializers;
    };
}