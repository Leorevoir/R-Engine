#pragma once

/**
 * Query Implementation
 */

template<typename... Wrappers>
r::ecs::Query<Wrappers...>::Query(Scene *scene, std::vector<Entity> entities) : _scene(scene), _entities(std::move(entities))
{
    /* __ctor__ */
}

template<typename... Wrappers>
u64 r::ecs::Query<Wrappers...>::size() const
{
    return static_cast<u64>(_entities.size());
}

/**
 * Iterator
 */

template<typename... Wrappers>
r::ecs::Query<Wrappers...>::Iterator::Iterator(Scene *scene, const std::vector<Entity> *list, u64 i) : _scene(scene), _list(list), _idx(i)
{
    /* __ctor__ */
}

template<typename... Wrappers>
bool r::ecs::Query<Wrappers...>::Iterator::operator!=(const Iterator &other) const
{
    return _idx != other._idx;
}

template<typename... Wrappers>
bool r::ecs::Query<Wrappers...>::Iterator::operator==(const Iterator &other) const
{
    return _idx == other._idx;
}

template<typename... Wrappers>
void r::ecs::Query<Wrappers...>::Iterator::operator++()
{
    ++_idx;
}

template<typename... Wrappers>
auto r::ecs::Query<Wrappers...>::Iterator::operator*() const
{
    Entity e = (*_list)[_idx];
    return std::tuple<Wrappers...>{build_wrapper<Wrappers>(_scene, e)...};
}

template<typename... Wrappers>
r::ecs::Entity r::ecs::Query<Wrappers...>::Iterator::entity() const
{
    return (*_list)[_idx];
}

template<typename... Wrappers>
template<typename W>
W r::ecs::Query<Wrappers...>::Iterator::build_wrapper(Scene *scene, Entity e)
{
    if constexpr (is_mut<W>::value) {
        using Comp = typename component_of<W>::type;
        return W{scene->get_component_ptr<Comp>(e)};
    } else if constexpr (is_ref<W>::value) {
        using Comp = typename component_of<W>::type;
        return W{scene->get_component_ptr<Comp>(e)};
    } else if constexpr (is_optional<W>::value) {
        using Comp = typename component_of<W>::type;
        return W{scene->get_component_ptr<Comp>(e)};
    } else if constexpr (is_with<W>::value || is_without<W>::value) {
        return W{};
    } else {
        static_assert(!sizeof(W), "r::ecs::Query wrappers must be Mut<T>, Ref<T>, With<T>, Without<T>, or Optional<T>");
    }
}

/**
 * Iteration
 */

template<typename... Wrappers>
typename r::ecs::Query<Wrappers...>::Iterator r::ecs::Query<Wrappers...>::begin() const
{
    return Iterator(_scene, &_entities, 0);
}

template<typename... Wrappers>
typename r::ecs::Query<Wrappers...>::Iterator r::ecs::Query<Wrappers...>::end() const
{
    return Iterator(_scene, &_entities, _entities.size());
}
