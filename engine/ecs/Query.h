#pragma once

#include <cstddef>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ComponentType.h"
#include "ComponentStorage.h"

namespace conductor {
    // =========================================================================
    // Query specifications
    // =========================================================================

    template<Component T>
    struct Read {
        using component_type = T;

        static constexpr bool writable = false;
    };

    template<Component T>
    struct Write {
        using component_type = T;

        static constexpr bool writable = true;
    };


    // =========================================================================
    // Query traits
    // =========================================================================

    template<typename T>
    struct IsQuerySpec : std::false_type {};

    template<Component T>
    struct IsQuerySpec<Read<T>> : std::true_type {};

    template<Component T>
    struct IsQuerySpec<Write<T>> : std::true_type {};

    template<typename T>
    inline constexpr bool IsQuerySpecV =
        IsQuerySpec<T>::value;

    template<typename Spec>
    struct QueryStorage;

    template<Component T>
    struct QueryStorage<Read<T>> {
        using type = const ComponentStorage<T>;
    };

    template<Component T>
    struct QueryStorage<Write<T>> {
        using type = ComponentStorage<T>;
    };

    template<typename Spec>
    using QueryStorageT =
        typename QueryStorage<Spec>::type;

    template<typename Spec>
    struct QueryReference;

    template<Component T>
    struct QueryReference<Read<T>> {
        using type = const T&;
    };

    template<Component T>
    struct QueryReference<Write<T>> {
        using type = T&;
    };

    template<typename Spec>
    using QueryReferenceT =
        typename QueryReference<Spec>::type;

    // =========================================================================
    // Query validation
    // =========================================================================

    template<typename... Specs>
    inline constexpr bool AllQuerySpecsValidV =
        (IsQuerySpecV<Specs> && ...);

    template<typename... Specs>
    struct AreUniqueComponents;

    template<>
    struct AreUniqueComponents<> : std::true_type {};

    template<typename First, typename... Rest>
    struct AreUniqueComponents<First, Rest...> {
    private:
        using ComponentType =
            typename First::component_type;

        template<typename Spec>
        static constexpr bool different =
            !std::is_same_v<
                ComponentType,
                typename Spec::component_type
            >;

    public:
        static constexpr bool value =
            (different<Rest> && ...)
            && AreUniqueComponents<Rest...>::value;
    };

    template<typename... Specs>
    inline constexpr bool AreUniqueComponentsV =
        AreUniqueComponents<Specs...>::value;

    // =========================================================================
    // Query
    // =========================================================================

    template<typename... Specs>
    class Query {
        static_assert(
            sizeof...(Specs) > 0,
            "Query requires at least one component"
        );

        static_assert(
            AllQuerySpecsValidV<Specs...>,
            "All Query types must be Read<T> or Write<T>"
        );

        static_assert(
            AreUniqueComponentsV<Specs...>,
            "A Query cannot contain the same component more than once"
        );

    private:
        using StorageTuple =
            std::tuple<QueryStorageT<Specs>*...>;

        using DriverSpec =
            std::tuple_element_t<
                0,
                std::tuple<Specs...>
            >;

        using DriverStorage =
            QueryStorageT<DriverSpec>;

        using DriverIterator =
            decltype(
                std::declval<DriverStorage&>().begin()
            );

        using AllIndices =
            std::index_sequence_for<Specs...>;

    public:
        // =====================================================================
        // Iterator
        // =====================================================================

        class Iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = std::tuple<QueryReferenceT<Specs>...>;
            using reference = value_type;
            using pointer = void;

            Iterator() noexcept = default;

            reference operator*() const noexcept {
                return make_result(
                    m_driver.entity()
                );
            }

            Iterator& operator++() noexcept {
                ++m_driver;
                skip_missing();
                return *this;
            }

            Iterator operator++(int) noexcept {
                Iterator copy = *this;
                ++(*this);
                return copy;
            }

            bool operator==(
                const Iterator& other
            ) const noexcept {
                return m_driver == other.m_driver;
            }

            bool operator!=(
                const Iterator& other
            ) const noexcept {
                return !(*this == other);
            }

        private:
            friend class Query;

            Iterator(
                DriverIterator driver,
                DriverIterator end,
                const StorageTuple* storages
            ) noexcept
                : m_driver(driver),
                  m_driver_end(end),
                  m_storages(storages) {
                skip_missing();
            }

            bool at_end() const noexcept {
                return m_driver == m_driver_end;
            }

            void skip_missing() noexcept {
                while (!at_end()) {
                    const Entity entity =
                        m_driver.entity();

                    if (matches(entity))
                        return;

                    ++m_driver;
                }
            }

            bool matches(
                Entity entity
            ) const noexcept {
                return matches_non_driver(
                    entity,
                    std::make_index_sequence<sizeof...(Specs) - 1>{}
                );
            }

            template<std::size_t... I>
            bool matches_non_driver(
                Entity entity,
                std::index_sequence<I...>
            ) const noexcept {
                return (
                    std::get<I + 1>(
                        *m_storages
                    )->contains(entity)
                    && ...
                );
            }

            reference make_result(
                Entity entity
            ) const noexcept {
                return make_result_impl(
                    entity,
                    AllIndices{}
                );
            }

            template<std::size_t... I>
            reference make_result_impl(
                Entity entity,
                std::index_sequence<I...>
            ) const noexcept {
                return {
                    *std::get<I>(
                        *m_storages
                    )->get(entity)...
                };
            }

            DriverIterator m_driver{};
            DriverIterator m_driver_end{};

            const StorageTuple* m_storages =
                nullptr;
        };

        // =====================================================================
        // Construction
        // =====================================================================

        explicit Query(
            QueryStorageT<Specs>&... storages
        ) noexcept
            : m_storages(&storages...) {}

        // =====================================================================
        // Range interface
        // =====================================================================

        Iterator begin() const noexcept {
            auto& driver =
                *std::get<0>(m_storages);

            return Iterator{
                driver.begin(),
                driver.end(),
                &m_storages
            };
        }

        Iterator end() const noexcept {
            auto& driver =
                *std::get<0>(m_storages);

            const auto end =
                driver.end();

            return Iterator{
                end,
                end,
                &m_storages
            };
        }

    private:
        StorageTuple m_storages;
    };
}