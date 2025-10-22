#pragma once

#include <R-Engine/ECS/Scene.hpp>

namespace r {

namespace ecs {

/**
 * wrappers for query parameters (Dependency Injection)
 */

/**
 * @brief Res
 * @info read-only global access to resource <T>
 * example: access to delta time, input state...
 */
template<typename T>
struct Res {
        using ResourceType = T;
        const T *ptr = nullptr;
};

/**
 * @brief ResMut
 * @info mutable global access to resource <T>
 * example: modifying a global game state resource
 */
template<typename T>
struct ResMut {
        using ResourceType = T;
        T *ptr = nullptr;
};

/**
 * @brief Mut
 * @info mutable access to component <T>
 * lets you write to a component (update position, change health...)
 */
template<typename T>
struct Mut {
        T *ptr = nullptr;
};

/**
 * @brief Ref
 * @info read-only access to component <T> of an Entity
 * lets you read a component (read position, check health...)
 */
template<typename T>
struct Ref {
        const T *ptr = nullptr;
};

/**
 * @brief With (Filter)
 * @info requires an entity to have component <T> without accessing its data.
 */
template<typename T>
struct With {
};

/**
 * @brief Without (Filter)
 * @info excludes entities that have component <T>.
 */
template<typename T>
struct Without {
};

/**
 * @brief Optional
 * @info provides conditional read-only access to component <T>.
 * The pointer will be nullptr if the entity does not have the component.
 */
template<typename T>
struct Optional {
        const T *ptr = nullptr;
};

/**
* traits
*/

template<typename>
struct is_res : std::false_type {
};

template<typename>
struct is_resmut : std::false_type {
};

template<typename>
struct is_mut : std::false_type {
};

template<typename>
struct is_ref : std::false_type {
};

template<typename>
struct is_with : std::false_type {
};

template<typename>
struct is_without : std::false_type {
};

template<typename>
struct is_optional : std::false_type {
};

template<typename T>
struct is_res<Res<T>> : std::true_type {
};

template<typename T>
struct is_resmut<ResMut<T>> : std::true_type {
};

template<typename T>
struct is_mut<Mut<T>> : std::true_type {
};

template<typename T>
struct is_ref<Ref<T>> : std::true_type {
};

template<typename T>
struct is_with<With<T>> : std::true_type {
};

template<typename T>
struct is_without<Without<T>> : std::true_type {
};

template<typename T>
struct is_optional<Optional<T>> : std::true_type {
};

/**
* helper to extract component type from wrapper
*/

template<typename W>
struct component_of;

template<typename T>
struct component_of<Mut<T>> {
        using type = T;
};

template<typename T>
struct component_of<Ref<T>> {
        using type = T;
};

template<typename T>
struct component_of<With<T>> {
        using type = T;
};

template<typename T>
struct component_of<Without<T>> {
        using type = T;
};

template<typename T>
struct component_of<Optional<T>> {
        using type = T;
};

/**
* @brief query
* @info accepts wrappers Mut<T> / Ref<T> / With<T> / Without<T> / Optional<T>
*
* void complex_system(Query<Mut<Position>, Without<Velocity>, Optional<Health>> q)
* {
*     for (auto [pos, _, health_opt] : q) {
*         // This system runs on entities that have Position, but not Velocity.
*         // Health is optional.
*         if (health_opt.ptr) {
*             // ...
*         }
*     }
* }
*/
template<typename... Wrappers>
struct Query {
    public:
        constexpr Query() = default;
        explicit Query(Scene *scene, std::vector<Entity> entities);

        /**
        * @brief iterator
        * @info iterates over entities, yielding tuples of wrappers
        */
        struct Iterator {
            public:
                explicit Iterator(Scene *scene, const std::vector<Entity> *list, u64 i);

                bool operator!=(const Iterator &other) const;
                bool operator==(const Iterator &other) const;
                void operator++();
                auto operator*() const;

                Entity entity() const;

                template<typename W>
                static W build_wrapper(Scene *scene, Entity e);

            private:
                Scene *_scene = nullptr;
                const std::vector<Entity> *_list;
                u64 _idx = 0;
        };

        Iterator begin() const;
        Iterator end() const;
        u64 size() const;

    private:
        Scene *_scene = nullptr;
        std::vector<Entity> _entities;
};

}// namespace ecs

}// namespace r

#include "Inline/Query.inl"
