#include "R-Engine/Plugins/TransformPlugin.hpp"

#include <R-Engine/Application.hpp>
#include <R-Engine/Components/Transform3d.hpp>
#include <R-Engine/ECS/Resolver.hpp>
#include <deque>
#include <unordered_map>
#include <vector>

namespace {///<< anonymous

using TransformWithoutGlobal = r::ecs::Query<r::ecs::With<r::Transform3d>, r::ecs::Without<r::GlobalTransform3d>>;
using Orphelins = r::ecs::Query<r::ecs::Without<r::ecs::Parent>>;

using TransformQuery =
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Mut<r::GlobalTransform3d>, r::ecs::Optional<r::ecs::Ref<r::ecs::Children>>>;

struct TransformCache {
    public:
        std::unordered_map<r::ecs::Entity, const r::Transform3d *> local;
        std::unordered_map<r::ecs::Entity, r::GlobalTransform3d *> global;
        std::unordered_map<r::ecs::Entity, const std::vector<r::ecs::Entity> *> children;
};

/**
* @brief startup system that adds missing GlobalTransform3d components
*/
static void transform_add_missing_global_system(r::ecs::Commands &commands, TransformWithoutGlobal query)
{
    for (auto it = query.begin(); it != query.end(); ++it) {
        commands.entity(it.entity()).insert(r::GlobalTransform3d{});
    }
}

/**
* @brief system that propagates local transforms to global transforms
* @details use a breadth-first search (BFS) approach
*/
static void transform_propagate_system(Orphelins roots_q, TransformQuery all_transforms_q)
{
    TransformCache cache;
    std::deque<r::ecs::Entity> queue;
    const u64 entity_count = all_transforms_q.size();

    if (entity_count == 0) {
        return;
    }

    cache.local.reserve(entity_count);
    cache.global.reserve(entity_count);
    cache.children.reserve(entity_count / 4);

    /**
    * @brief build the cache
    * @details use an iterator loop to access the entity ID via it.entity()
    */
    for (auto it = all_transforms_q.begin(); it != all_transforms_q.end(); ++it) {
        const auto [local, global, children_opt] = *it;
        const r::ecs::Entity entity = it.entity();

        cache.local[entity] = local.ptr;
        cache.global[entity] = global.ptr;

        if (children_opt.ptr) {
            cache.children[entity] = &children_opt.ptr->ptr->entities;
        }
    }

    /**
    * @brief initialize the queue with root entities
    * @details use an iterator loop to get the entity ID
    */
    for (auto it = roots_q.begin(); it != roots_q.end(); ++it) {
        const r::ecs::Entity entity = it.entity();
        const auto local_it = cache.local.find(entity);

        if (local_it == cache.local.end()) {
            continue;
        }

        const r::Transform3d *local = local_it->second;
        r::GlobalTransform3d *global = cache.global.at(entity);

        global->position = local->position;
        global->rotation = local->rotation;
        global->scale = local->scale;

        queue.push_back(entity);
    }

    /**
    * @brief BFS loop to propagate transforms
    * @details foreach entity in the queue -> update its children's global transforms
    */
    while (!queue.empty()) {
        const r::ecs::Entity parent_entity = queue.front();
        queue.pop_front();

        const auto children_it = cache.children.find(parent_entity);
        if (children_it == cache.children.end()) {
            continue;
        }

        const r::GlobalTransform3d &parent_global = *cache.global.at(parent_entity);
        const std::vector<r::ecs::Entity> &child_entities = *children_it->second;

        for (const r::ecs::Entity child_entity : child_entities) {
            const r::Transform3d *child_local = cache.local.at(child_entity);
            r::GlobalTransform3d *child_global = cache.global.at(child_entity);

            *child_global = r::GlobalTransform3d::from_local_and_parent(*child_local, parent_global);

            queue.push_back(child_entity);
        }
    }
}

}// namespace

void r::TransformPlugin::build(Application &app)
{
    app.add_systems<transform_add_missing_global_system>(r::Schedule::UPDATE)
        .add_systems<transform_propagate_system>(r::Schedule::BEFORE_RENDER_2D);
}
