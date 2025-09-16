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
        const T *ptr = nullptr;
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
* traits
*/

template<typename>
struct is_res : std::false_type {
};

template<typename>
struct is_mut : std::false_type {
};

template<typename>
struct is_ref : std::false_type {
};

template<typename T>
struct is_res<Res<T>> : std::true_type {
};

template<typename T>
struct is_mut<Mut<T>> : std::true_type {
};

template<typename T>
struct is_ref<Ref<T>> : std::true_type {
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

/**
* @brief query
* @info accepts wrappers Mut<T> / Ref<T>
*
* void move_system(Res<FrameTime> time, Query<Mut<Position>, Ref<Velocity>> q)
* {
*     for (const auto &tup : q) {
*         pos.ptr->x += vel.ptr->vx * time.ptr->delta_time;
*         pos.ptr->y += vel.ptr->vy * time.ptr->delta_time;
*     }
* }
*
* in this example:
*   - the ECS injects FrameTime (a global resource)
*   - ECS finds all entities with Position and Velocity components
*   - for each entity, ECS yields a tuple (Mut<Position>, Ref<Velocity>) to the system
*   - the system can mutate Position and read Velocity
*/
template<typename... Wrappers>
struct Query {
    public:
        constexpr Query() = default;
        explicit Query(Scene *scene, std::vector<Entity> entities) : _scene(scene), _entities(std::move(entities))
        {
            /* __ctor__ */
        }

        /**
        * @brief iterator
        * @info iterates over entities, yielding tuples of wrappers
        */
        struct Iterator {
            public:
                explicit Iterator(Scene *scene, const std::vector<Entity> *list, u64 i) : _scene(scene), _list(list), _idx(i)
                {
                    /* __ctor__ */
                }

                bool operator!=(const Iterator &other) const
                {
                    return _idx != other._idx;
                }

                bool operator==(const Iterator &other) const
                {
                    return _idx == other._idx;
                }

                void operator++()
                {
                    ++_idx;
                }

                auto operator*() const
                {
                    Entity e = (*_list)[_idx];

                    return std::tuple<Wrappers...>{build_wrapper<Wrappers>(_scene, e)...};
                }

                template<typename W>
                static W build_wrapper(Scene *scene, Entity e)
                {
                    if constexpr (is_mut<W>::value) {
                        using Comp = typename component_of<W>::type;
                        return W{scene->get_component_ptr<Comp>(e)};
                    } else if constexpr (is_ref<W>::value) {
                        using Comp = typename component_of<W>::type;
                        return W{scene->get_component_ptr<Comp>(e)};
                    } else {
                        static_assert(false, "Query wrappers must be Mut<T> or Ref<T>");
                    }
                }

            private:
                Scene *_scene = nullptr;
                const std::vector<Entity> *_list;
                u64 _idx = 0;
        };

        Iterator begin() const
        {
            return Iterator(_scene, &_entities, 0);
        }

        Iterator end() const
        {
            return Iterator(_scene, &_entities, _entities.size());
        }

    private:
        Scene *_scene = nullptr;
        std::vector<Entity> _entities;
};

}// namespace ecs

}// namespace r
