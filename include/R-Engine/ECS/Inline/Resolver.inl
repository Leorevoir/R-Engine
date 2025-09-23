#pragma once

#include "R-Engine/ECS/Resolver.hpp"
#include <typeindex>

template<typename T>
r::ecs::Res<T> r::ecs::Resolver::resolve(std::type_identity<r::ecs::Res<T>>)
{
    r::ecs::Res<T> r;

    r.ptr = _scene->get_resource_ptr<T>();
    return r;
}

template<typename... Wrappers>
r::ecs::Resolver::Q<Wrappers...> r::ecs::Resolver::resolve(std::type_identity<r::ecs::Query<Wrappers...>>)
{
    std::vector<Entity> result_entities;
    const auto &archetypes = _scene->get_archetypes();

    std::vector<std::type_index> required_types;
    std::vector<std::type_index> excluded_types;
    (this->_collect_component_types<Wrappers>(required_types, excluded_types), ...);

    for (const auto &archetype : archetypes) {
        if (archetype.table.entities.empty()) {
            continue;
        }

        bool match = true;

        for (const auto &req_type : required_types) {
            if (!archetype.has_component(req_type)) {
                match = false;
                break;
            }
        }
        if (!match)
            continue;

        for (const auto &excl_type : excluded_types) {
            if (archetype.has_component(excl_type)) {
                match = false;
                break;
            }
        }
        if (!match)
            continue;

        result_entities.insert(result_entities.end(), archetype.table.entities.begin(), archetype.table.entities.end());
    }

    return Q<Wrappers...>(_scene, std::move(result_entities));
}

template<typename T>
T r::ecs::Resolver::resolve(std::type_identity<T>)
{
    static_assert(!std::is_same_v<T, T>,
        "r::ecs::Resolver::resolve: Unsupported system parameter type. Use Res<T>, Query<...>, or Commands.");
    return T{};
}

/**
 * private
 */
template<typename W>
void r::ecs::Resolver::_collect_component_types(
    std::vector<std::type_index> &required, std::vector<std::type_index> &excluded)
{
    if constexpr (is_mut<W>::value || is_ref<W>::value || is_with<W>::value) {
        using Comp = typename component_of<W>::type;
        required.push_back(typeid(Comp));
    } else if constexpr (is_without<W>::value) {
        using Comp = typename component_of<W>::type;
        excluded.push_back(typeid(Comp));
    }
}
