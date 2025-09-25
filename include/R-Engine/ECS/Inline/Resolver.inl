#pragma once

#include "R-Engine/ECS/Resolver.hpp"
#include <algorithm>
#include <unordered_set>

/**
 * Resolver Implementation
 */

/**
 * Res<T>
 */
template<typename T>
r::ecs::Res<T> r::ecs::Resolver::resolve(std::type_identity<r::ecs::Res<T>>)
{
    r::ecs::Res<T> r;

    r.ptr = _scene->get_resource_ptr<T>();
    return r;
}

/**
 * ResMut<T>
 */
template<typename T>
r::ecs::ResMut<T> r::ecs::Resolver::resolve(std::type_identity<r::ecs::ResMut<T>>)
{
    r::ecs::ResMut<T> r;

    r.ptr = _scene->get_resource_ptr<T>();
    return r;
}

/**
 * Query<Wrappers...>
 */
template<typename... Wrappers>
r::ecs::Resolver::Q<Wrappers...> r::ecs::Resolver::resolve(std::type_identity<r::ecs::Query<Wrappers...>>)
{
    // --- Stage 1: Collect entity lists for required components and a set for excluded entities ---
    std::vector<std::vector<Entity>> required_lists;
    required_lists.reserve(sizeof...(Wrappers));
    (this->_collect_required_for<Wrappers>(required_lists), ...);

    std::unordered_set<Entity> exclusion_set;
    (this->_collect_excluded_for<Wrappers>(exclusion_set), ...);

    // If no required components are specified, we cannot form a base set of entities.
    // An advanced ECS might iterate all entities, but this one is component-driven.
    if (required_lists.empty()) {
        return Q<Wrappers...>(_scene, {});
    }

    // --- Stage 2: Intersect all required lists to get a base set of entities ---

    // if any required list is empty -> no matches
    for (auto const &l : required_lists) {
        if (l.empty()) {
            return Q<Wrappers...>(_scene, {});
        }
    }

    // pick index of smallest list for optimization
    u64 best_idx = 0;
    for (u64 i = 1; i < required_lists.size(); ++i) {
        if (required_lists[i].size() < required_lists[best_idx].size())
            best_idx = i;
    }

    // prepare optional hashed containers for large lists to speed up lookups
    constexpr u64 hash_threshold = 64;
    std::vector<std::optional<std::unordered_set<Entity>>> hashed(required_lists.size());
    for (u64 i = 0; i < required_lists.size(); ++i) {
        if (i == best_idx)
            continue;
        if (required_lists[i].size() > hash_threshold) {
            hashed[i].emplace(required_lists[i].begin(), required_lists[i].end());
        }
    }

    // iterate smallest list and test membership in others
    std::vector<Entity> base_entities;
    base_entities.reserve(required_lists[best_idx].size());

    for (Entity e : required_lists[best_idx]) {
        bool ok = true;
        for (u64 j = 0; j < required_lists.size(); ++j) {
            if (j == best_idx)
                continue;
            if (hashed[j]) {
                if (hashed[j]->find(e) == hashed[j]->end()) {
                    ok = false;
                    break;
                }
            } else {
                if (std::find(required_lists[j].begin(), required_lists[j].end(), e) == required_lists[j].end()) {
                    ok = false;
                    break;
                }
            }
        }
        if (ok)
            base_entities.push_back(e);
    }

    // --- Stage 3: Filter the base set using the exclusion set ---
    if (exclusion_set.empty()) {
        return Q<Wrappers...>(_scene, std::move(base_entities));
    }

    std::vector<Entity> final_result;
    final_result.reserve(base_entities.size());
    for (Entity e : base_entities) {
        if (exclusion_set.find(e) == exclusion_set.end()) {
            final_result.push_back(e);
        }
    }

    return Q<Wrappers...>(_scene, std::move(final_result));
}

/**
 * Fallback for unsupported types
 */
template<typename T>
T r::ecs::Resolver::resolve(std::type_identity<T>)
{
    static_assert(!std::is_same_v<T, T>,
        "r::ecs::Resolver::resolve: Unsupported system parameter type. Use Res<T>, ResMut<T>, Query<...>, or Commands.");
    return T{};
}

/**
 * private
 */
template<typename W>
void r::ecs::Resolver::_collect_required_for(std::vector<std::vector<Entity>> &out)
{
    if constexpr (is_mut<W>::value || is_ref<W>::value || is_with<W>::value) {
        using Comp = typename component_of<W>::type;
        auto key = std::type_index(typeid(Comp));
        const auto &storages = _scene->get_storages();
        const auto it = storages.find(key);

        if (it == storages.end()) {
            out.emplace_back();// Component storage doesn't exist, push an empty list.
            return;
        }
        out.push_back(it->second->entity_list());
    }
}

template<typename W>
void r::ecs::Resolver::_collect_excluded_for(std::unordered_set<Entity> &out)
{
    if constexpr (is_without<W>::value) {
        using Comp = typename component_of<W>::type;
        auto key = std::type_index(typeid(Comp));
        const auto &storages = _scene->get_storages();
        const auto it = storages.find(key);

        if (it != storages.end()) {
            const auto list = it->second->entity_list();
            out.insert(list.begin(), list.end());
        }
    }
}
