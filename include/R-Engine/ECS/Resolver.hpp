#pragma once

#include <R-Engine/Core/Error.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/ECS/Scene.hpp>

#include <algorithm>
#include <unordered_set>

namespace r {
namespace ecs {

struct Resolver {
    public:
        template<typename... Wrappers>
        using Q = Query<Wrappers...>;

        explicit Resolver(Scene *s);

        /** 
         * @brief Res<T>
         */
        template<typename T>
        Res<T> resolve(std::type_identity<Res<T>>)
        {
            Res<T> r;
            r.ptr = _scene->get_resource_ptr<T>();
            return r;
        }

        /**
        * @brief Commands
        */
        Commands resolve(std::type_identity<Commands>);

        /**
        * @brief Query<Wrappers...>
        */
        template<typename... Wrappers>
        Q<Wrappers...> resolve(std::type_identity<Query<Wrappers...>>)
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
         * @brief fallback for unsupported types
         */
        template<typename T>
        T resolve(std::type_identity<T>)
        {
            static_assert(!std::is_same_v<T, T>,
                "Resolver::resolve: Unsupported system parameter type. Use Res<T>, Query<Mut<T>/Ref<T>...>, or Commands.");

            return T{};
        }

    private:
        Scene *_scene;

        /**
        * @brief collect list for wrapper W (must be Mut<T> or Ref<T>)
        */
        template<typename W>
        void _collect_for(std::vector<std::vector<Entity>> &out)
        {
            static_assert(is_mut<W>::value || is_ref<W>::value, "Query arguments must be Mut<T> or Ref<T>");

            using Comp = typename component_of<W>::type;
            const u64 id = type_id<Comp>();
            const auto &storages = _scene->getStorages();
            const auto it = storages.find(id);

            if (it == storages.end()) {
                out.emplace_back();
                return;
            }

            out.push_back(it->second->entity_list());
        }
};

}// namespace ecs
}// namespace r
