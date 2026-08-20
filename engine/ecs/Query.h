#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ComponentType.h"
#include "ComponentView.h"

namespace conductor {
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


    namespace detail {
        // Access Traits

        template<typename Access>
        using access_component_t =
            typename Access::component_type;

        // View traits

        template<typename View>
        using view_type_t =
            std::remove_cvref_t<View>;

        template<typename View>
        using view_component_t =
            typename view_type_t<View>::component_type;

        template<typename View>
        inline constexpr bool view_is_const_v =
            std::is_const_v<
                typename view_type_t<View>::storage_type
            >;

        // Validate an Access/View pair

        template<typename Access, typename View>
        inline constexpr bool valid_access_view_v =
            std::is_same_v<
                access_component_t<Access>,
                std::remove_cv_t<
                    view_component_t<View>
                >
            >
            &&
            (
                !Access::writable ||
                !view_is_const_v<View>
            );

        // Query result reference

        template<typename Access>
        using reference_t =
            std::conditional_t<
                Access::writable,
                typename Access::component_type&,
                const typename Access::component_type&
            >;

        template<typename... Access>
        using reference_tuple_t = 
            std::tuple<
                reference_t<Access>...
            >;

        // Get a component from a view

        template<typename Access, typename View>
        decltype(auto) get_component(
            View& view,
            Entity entity
        ) {
            using Component = 
                typename Access::component_type;

            auto *component =
                view.get(entity);

            assert(component != nullptr);

            if constexpr(Access::writable) {
                return *component;
            } else {
                return static_cast<const Component&>(
                    *component
                );
            }
        }
    }

    // =====================================================
    // Query
    // 
    // ViewTuple contains the concrete ComponentView types.
    //
    // Access... describes what the query exposes.
    // =====================================================
    template<typename ViewTuple, typename... Access>
    class Query {
        static_assert(
            sizeof...(Access) > 0,
            "Query must contain at least one access type"
        );

        static_assert(
            std::tuple_size_v<ViewTuple> == 
            sizeof...(Access),
            "Query requires one ComponentView per access"
        );

    private:
        using AccessTuple =
            std::tuple<Access...>;

        using DriverView =
            std::tuple_element_t<0, ViewTuple>;

        using DriverIterator =
            decltype(
                std::declval<DriverView&>().begin()
            );

        static constexpr size_t ViewCount =
            sizeof...(Access);

        // Validate every Access/View pair

        template<size_t... Index>
        static consteval bool validate_views_impl(
            std::index_sequence<Index...>
        ) {
            return (
                detail::valid_access_view_v<
                    std::tuple_element_t<Index, AccessTuple>,
                    std::tuple_element_t<Index, ViewTuple>
                >
                && ...
            );
        }

        static consteval bool validate_views() {
            return validate_views_impl(
                std::make_index_sequence<ViewCount>{}
            );
        }

        static_assert(
            validate_views(),
            "Query access types do not match supplied ComponentViews"
        );

    public:
        using reference = 
            detail::reference_tuple_t<Access...>;

        // Query Iterator
        class Iterator {
        public:
            using iterator_category =
                std::forward_iterator_tag;

            /*
             * A query dereference produces a tuple of references.
             */
            using value_type = reference;
            using difference_type = std::ptrdiff_t;
            using pointer = void;
            using reference_type = reference;

            Iterator() noexcept = default;

            reference_type operator*() const {
                return m_query->dereference(m_iterator);
            }

            Iterator& operator++() {
                ++m_iterator;

                m_query->advance(m_iterator);

                return *this;
            }

            Iterator operator++(int) {
                Iterator copy = *this;
                ++(*this);
                return copy;
            }

            bool operator==(const Iterator& other) const noexcept {
                return m_query == other.m_query &&
                       m_iterator == other.m_iterator;
            }

            bool operator!=(const Iterator& other) const noexcept {
                return !(*this == other);
            }

            Entity entity() const noexcept {
                return m_iterator.entity();
            }

        private:
            friend class Query;

            Iterator(
                Query *query,
                DriverIterator iterator
            ) noexcept
                : m_query(query),
                  m_iterator(std::move(iterator)) {}

            Query *m_query = nullptr;
            DriverIterator m_iterator;
        };

    public:
        // Construction

        explicit Query(ViewTuple views)
            : m_views(std::move(views)) {}

        // Iteration

        Iterator begin() {
            auto iterator =
                std::get<0>(m_views).begin();

            advance(iterator);

            return Iterator(
                this,
                std::move(iterator)
            );
        }

        Iterator end() {
            return Iterator(
                this,
                std::get<0>(m_views).end()
            );
        }

        bool empty() {
            return begin() == end();
        }

        // =======================================================
        // Number of entities in the driver view.
        //
        // This is intentionally not called size(), because it is
        // not necessarily the number of matching entities.
        // =======================================================
        size_t driver_size() const noexcept {
            return std::get<0>(m_views).size();
        }

    private:
        // ========================================================
        // Determine whether an entity exists in all views.
        //
        // The driver view does not need to be checked because the 
        // driver iterator necessarily points at an entity that
        // exists in it.
        // ========================================================
        template<size_t... Index>
        bool matches_impl(
            Entity entity,
            std::index_sequence<Index...>
        ) const noexcept {
            return (
                std::get<Index + 1>(m_views).contains(entity)
                && ...
            );
        }

        bool matches(Entity entity) const noexcept {
            // Skip view 0 because it is the driver
            if constexpr(ViewCount == 1) {
                return true;
            } else {
                return matches_impl(
                    entity,
                    std::make_index_sequence<
                        ViewCount - 1
                    >{}
                );
            }
        }

        // Advance the driver iterator to the next matching entity
        void advance(DriverIterator& iterator) {
            const auto end =
                std::get<0>(m_views).end();

            while(iterator != end) {
                const Entity entity =
                    iterator.entity();

                if(matches(entity))
                    return;

                ++iterator;
            }
        }

        // Dereference

        template<size_t... Index>
        reference dereference_impl(
            Entity entity,
            std::index_sequence<Index...>
        ) {
            return reference{
                detail::get_component<
                    std::tuple_element_t<
                        Index,
                        AccessTuple
                    >
                >(
                    std::get<Index>(m_views),
                    entity
                )...
            };
        }

        reference dereference(
            const DriverIterator& iterator
        ) {
            const Entity entity =
                iterator.entity();

            return dereference_impl(
                entity,
                std::make_index_sequence<ViewCount>{}
            );
        }

        ViewTuple m_views;
    };

    // ================================
    // make_query
    // ================================
    template<typename... QueryAccess, typename... Views>
    requires(sizeof...(QueryAccess) == sizeof...(Views))
    auto make_query(Views&&... views) {
        using ViewTuple =
            std::tuple<std::remove_cvref_t<Views>...>;

        using QueryType =
            Query<ViewTuple, QueryAccess...>;

        return QueryType(
            ViewTuple(
                std::forward<Views>(views)...
            )
        );
    }
}