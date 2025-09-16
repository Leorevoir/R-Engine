#pragma once

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
 * Query<Wrappers...>
 */
template<typename... Wrappers>
r::ecs::Resolver::Q<Wrappers...> r::ecs::Resolver::resolve(std::type_identity<r::ecs::Query<Wrappers...>>)
{
    std::vector<std::vector<Entity>> lists;
    lists.reserve(sizeof...(Wrappers));

    /** collect lists for each wrapper */
    (this->_collect_for<Wrappers>(lists), ...);

    /** if any list empty -> no matches */
    for (auto const &l : lists) {
        if (l.empty()) {
            return Q<Wrappers...>(_scene, {});
        }
    }

    /** pick index of smallest list */
    u64 best_idx = 0;
    for (u64 i = 1; i < lists.size(); ++i) {
        if (lists[i].size() < lists[best_idx].size())
            best_idx = i;
    }

    /** prepare optional hashed containers for large lists to speed up lookups */
    constexpr u64 hash_threshold = 64;
    std::vector<std::optional<std::unordered_set<Entity>>> hashed(lists.size());
    for (u64 i = 0; i < lists.size(); ++i) {
        if (i == best_idx)
            continue;
        if (lists[i].size() > hash_threshold) {
            hashed[i].emplace(lists[i].begin(), lists[i].end());
        }
    }

    /** iterate smallest list and test membership in others */
    std::vector<Entity> result;
    result.reserve(lists[best_idx].size());

    for (Entity e : lists[best_idx]) {
        bool ok = true;
        for (u64 j = 0; j < lists.size(); ++j) {
            if (j == best_idx)
                continue;
            if (hashed[j]) {
                if (hashed[j]->find(e) == hashed[j]->end()) {
                    ok = false;
                    break;
                }
            } else {
                if (std::find(lists[j].begin(), lists[j].end(), e) == lists[j].end()) {
                    ok = false;
                    break;
                }
            }
        }
        if (ok)
            result.push_back(e);
    }

    return Q<Wrappers...>(_scene, std::move(result));
}

/**
 * Fallback for unsupported types
 */
template<typename T>
T r::ecs::Resolver::resolve(std::type_identity<T>)
{
    static_assert(!std::is_same_v<T, T>,
        "r::ecs::Resolver::resolve: Unsupported system parameter type. Use Res<T>, Query<Mut<T>/Ref<T>...>, or Commands.");
    return T{};
}

/**
 * private
 */

/**
 * _collect_for wrapper
 */
template<typename W>
void r::ecs::Resolver::_collect_for(std::vector<std::vector<Entity>> &out)
{
    static_assert(is_mut<W>::value || is_ref<W>::value, "Query arguments must be Mut<T> or Ref<T>");

    using Comp = typename component_of<W>::type;
    auto key = std::type_index(typeid(Comp));
    const auto &storages = _scene->getStorages();
    const auto it = storages.find(key);

    if (it == storages.end()) {
        out.emplace_back();
        return;
    }

    out.push_back(it->second->entity_list());
}
