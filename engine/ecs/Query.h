#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ComponentType.h"
#include "ComponentView.h"
#include "core/Executor.h"

namespace conductor {
    // ============================================================
    // Access Types
    // ============================================================

    enum class AccessMode {
        Read,
        Write
    };

    struct ComponentAccess {
        ComponentTypeId component;
        AccessMode mode;
    };
    
    // ============================================================
    // Access declarations
    // ============================================================

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
        // ========================================================
        // Access traits
        // ========================================================

        template<typename Access>
        using access_component_t =
            typename Access::component_type;

        template<typename Access>
        inline constexpr bool access_is_writable_v =
            Access::writable;

        // ========================================================
        // View traits
        // ========================================================

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

        // ========================================================
        // Validate an Access/View pair
        // ========================================================

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
                !access_is_writable_v<Access> ||
                !view_is_const_v<View>
            );

        // ========================================================
        // Query result reference
        // ========================================================

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

        // ========================================================
        // Get a component from a view
        // ========================================================

        template<typename Access, typename View>
        decltype(auto) get_component(
            View& view,
            Entity entity
        ) {
            using Component =
                typename Access::component_type;

            auto* component =
                view.get(entity);

            assert(component != nullptr);

            if constexpr(Access::writable) {
                return *component;
            }
            else {
                return static_cast<const Component&>(
                    *component
                );
            }
        }

        // ========================================================
        // Convert a storage to its view
        //
        // This allows:
        //
        //     Query<Read<Position>>::bind(storage)
        //
        // as well as:
        //
        //     Query<Read<Position>>::bind(storage.view())
        // ========================================================

        template<typename T>
        concept HasView =
            requires(T& storage) {
                storage.view();
            };

        template<typename T>
        auto make_view(T& storage) {
            return storage.view();
        }

        // ========================================================
        // Query validation
        // ========================================================

        template<typename T>
        struct is_query_access : std::false_type {};

        template<typename T>
        struct is_query_access<Read<T>>
            : std::bool_constant<
                Component<T>
            > {};

        template<typename T>
        struct is_query_access<Write<T>>
            : std::bool_constant<
                Component<T>
            > {};

        template<typename T>
        inline constexpr bool is_query_access_v =
            is_query_access<T>::value;

        template<typename... Access>
        inline constexpr bool unique_components_v = [] {
            using Components =
                std::tuple<
                    typename Access::component_type...
                >;

            return []<std::size_t... I>(
                std::index_sequence<I...>
            ) {
                return []<std::size_t... J>(
                    std::index_sequence<J...>
                ) {
                    return (
                        (
                            I == J ||
                            !std::is_same_v<
                                std::tuple_element_t<I, Components>,
                                std::tuple_element_t<J, Components>
                            >
                        )
                        && ...
                    );
                }(
                    std::make_index_sequence<
                        sizeof...(Access)
                    >{}
                );
            }(
                std::make_index_sequence<
                    sizeof...(Access)
                >{}
            );
        }();

        template<typename Access>
        struct is_read_access
            : std::bool_constant<!Access::writable> {};

        template<typename Access>
        struct is_write_access
            : std::bool_constant<Access::writable> {};

        template<typename... Access>
        inline constexpr bool valid_query_accesses_v =
            (
                is_query_access<Access>::value
                && ...
            )
            &&
            unique_components_v<Access...>;
    }

    // ============================================================
    // Query
    //
    // Example:
    //
    // using MovementQuery =
    //     Query<
    //         Read<Velocity>,
    //         Write<Position>
    //     >;
    //
    // Runtime binding:
    //
    // auto query =
    //     MovementQuery::bind(
    //         velocity.view(),
    //         position.view()
    //     );
    //
    // or:
    //
    // auto query =
    //     MovementQuery::bind(
    //         velocity,
    //         position
    //     );
    // ============================================================

    template<typename... Access>
    class Query {
        static_assert(
            sizeof...(Access) > 0,
            "Query must contain at least one access type"
        );

        static_assert(
            detail::valid_query_accesses_v<Access...>,
            "Query contains invalid, duplicate, or non-component access types"
        );

    public:
        template<typename ViewTuple>
        class BoundQuery;

        template<typename... Views>
        using Bound =
            BoundQuery<
                std::tuple<
                    std::remove_cvref_t<Views>...
                >
            >;

        using access_types = 
            std::tuple<Access...>;

    private:
        using AccessTuple =
            std::tuple<Access...>;

    public:
        using reference =
            detail::reference_tuple_t<Access...>;

    public:
        // ========================================================
        // Bound query implementation
        // ========================================================

        template<typename ViewTuple>
        class BoundQuery {
        public:
            using query_type = Query;
        
        private:
            using DriverView =
                std::tuple_element_t<
                    0,
                    ViewTuple
                >;

            using DriverIterator =
                decltype(
                    std::declval<
                        DriverView&
                    >().begin()
                );

            static constexpr std::size_t ViewCount =
                sizeof...(Access);

        public:
            class Iterator {
            public:
                using iterator_category =
                    std::forward_iterator_tag;

                using value_type =
                    reference;

                using difference_type =
                    std::ptrdiff_t;

                using pointer =
                    void;

                using reference_type =
                    reference;

                Iterator() noexcept = default;

                reference_type operator*() const {
                    return m_query->dereference(
                        m_iterator
                    );
                }

                Iterator& operator++() {
                    ++m_iterator;

                    m_query->advance(
                        m_iterator
                    );

                    return *this;
                }

                Iterator operator++(int) {
                    Iterator copy = *this;

                    ++(*this);

                    return copy;
                }

                bool operator==(
                    const Iterator& other
                ) const noexcept {
                    return
                        m_query == other.m_query &&
                        m_iterator == other.m_iterator;
                }

                bool operator!=(
                    const Iterator& other
                ) const noexcept {
                    return !(*this == other);
                }

                Entity entity() const noexcept {
                    return m_iterator.entity();
                }

            private:
                friend class BoundQuery;

                Iterator(
                    BoundQuery* query,
                    DriverIterator iterator
                ) noexcept
                    : m_query(query),
                      m_iterator(
                          std::move(iterator)
                      ) {}

                BoundQuery* m_query = nullptr;

                DriverIterator m_iterator;
            };

            explicit BoundQuery(
                ViewTuple views
            )
                : m_views(
                    std::move(views)
                ) {}

            Iterator begin() {
                auto iterator =
                    std::get<0>(
                        m_views
                    ).begin();

                advance(iterator);

                return Iterator(
                    this,
                    std::move(iterator)
                );
            }

            Iterator end() {
                return Iterator(
                    this,
                    std::get<0>(
                        m_views
                    ).end()
                );
            }

            bool empty() {
                return begin() == end();
            }

            std::size_t driver_size() const noexcept {
                return std::get<0>(
                    m_views
                ).size();
            }

            template<typename Func>
            requires std::is_invocable_v<Func, Entity&, detail::reference_t<Access>...>
            void iter(Func&& lambda) {
                for(auto it = begin(); it != end(); it++) {
                    Entity current_entity = it.entity();

                    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                        std::forward<Func>(lambda)(
                            current_entity,
                            detail::get_component<
                                std::tuple_element_t<Is, std::tuple<Access...>>
                            >(std::get<Is>(m_views), current_entity)...
                        );
                    }(std::make_index_sequence<sizeof...(Access)>{});
                }
            }

            template<typename Func>
            requires std::is_invocable_v<Func, Entity&, detail::reference_t<Access>...>
            void par_iter(Executor& executor, Func&& lambda) {
                const auto size = driver_size();
                
                if(size < 4096) {
                    iter(std::forward<Func>(lambda));
                    return;
                }

                const std::size_t task_count =
                    executor.worker_count() * 8;

                const std::size_t chunk_size =
                    (size + task_count - 1) / task_count;

                std::vector<std::function<void()>> tasks;
                tasks.reserve(task_count);

                for(std::size_t begin = 0; begin < driver_size(); begin += chunk_size) {
                    const std::size_t end = 
                        std::min(begin + chunk_size, size);

                    tasks.emplace_back([&, begin, end]() {
                        for(std::size_t i = begin; i < end; i++) {
                            Entity *current_entity = std::get<0>(m_views).entity_at(i);

                            if(current_entity != nullptr && matches(*current_entity)) {
                                [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                                    lambda(
                                        *current_entity,
                                        detail::get_component<
                                            std::tuple_element_t<Is, std::tuple<Access...>>
                                        >(std::get<Is>(m_views), *current_entity)...
                                    );
                                }(std::make_index_sequence<sizeof...(Access)>{});
                            }
                        }
                    });
                }

                executor.parallel(std::move(tasks));
            }

        private:
            // ====================================================
            // Matching
            // ====================================================

            template<std::size_t... Index>
            bool matches_impl(
                Entity entity,
                std::index_sequence<Index...>
            ) const noexcept {
                return (
                    std::get<Index + 1>(
                        m_views
                    ).contains(entity)
                    && ...
                );
            }

            bool matches(
                Entity entity
            ) const noexcept {
                if constexpr(ViewCount == 1) {
                    return true;
                }
                else {
                    return matches_impl(
                        entity,
                        std::make_index_sequence<
                            ViewCount - 1
                        >{}
                    );
                }
            }

            // ====================================================
            // Advance
            // ====================================================

            void advance(
                DriverIterator& iterator
            ) {
                const auto end =
                    std::get<0>(
                        m_views
                    ).end();

                while(iterator != end) {

                    const Entity entity =
                        iterator.entity();


                    if(matches(entity))
                        return;


                    ++iterator;
                }
            }

            // ====================================================
            // Dereference
            // ====================================================

            template<std::size_t... Index>
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
                        std::get<Index>(
                            m_views
                        ),
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
                    std::make_index_sequence<
                        ViewCount
                    >{}
                );
            }

            ViewTuple m_views;
        };

    public:
        // ========================================================
        // Bind
        // ========================================================

        template<typename... Views>
        requires(
            sizeof...(Views) == sizeof...(Access)
        )
        static auto bind(
            Views&&... views
        ) {
            using ViewTuple =
                std::tuple<
                    std::remove_cvref_t<Views>...
                >;

            static_assert(
                validate_views<ViewTuple>(),
                "Query access types do not match supplied ComponentViews"
            );

            return BoundQuery<ViewTuple>(
                ViewTuple(
                    std::forward<Views>(views)...
                )
            );
        }

        // ========================================================
        // Bind using ComponentStorage directly
        //
        // This overload is intentionally provided as a convenience
        // for the existing ComponentStorage API.
        // ========================================================

        template<typename... Storage>
        requires(
            sizeof...(Storage) == sizeof...(Access)
            &&
            (
                detail::HasView<
                    Storage
                >
                && ...
            )
        )
        static auto bind_storage(
            Storage&... storage
        ) {
            return bind(
                storage.view()...
            );
        }

    private:
        // ========================================================
        // Validate views
        // ========================================================

        template<typename ViewTuple, std::size_t... Index>
        static consteval bool validate_views_impl(
            std::index_sequence<Index...>
        ) {
            return (
                detail::valid_access_view_v<
                    std::tuple_element_t<
                        Index,
                        AccessTuple
                    >,
                    std::tuple_element_t<
                        Index,
                        ViewTuple
                    >
                >
                && ...
            );
        }

        template<typename ViewTuple>
        static consteval bool validate_views() {

            return validate_views_impl<ViewTuple>(
                std::make_index_sequence<
                    sizeof...(Access)
                >{}
            );
        }
    };

    // ============================================================
    // Query concept
    // ============================================================

    template<typename T>
    concept QueryType =
        requires {
            typename T::reference;
        };
}