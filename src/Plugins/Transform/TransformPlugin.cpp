#include "R-Engine/Plugins/TransformPlugin.hpp"

#include <R-Engine/Application.hpp>
#include <R-Engine/Components/Transform3d.hpp>
#include <R-Engine/ECS/Resolver.hpp>
#include <deque>
#include <unordered_map>
#include <vector>

namespace {// anonymous namespace

// clang-format off

using TransformWithoutGlobal = r::ecs::Query<
        r::ecs::With<r::Transform3d>,
        r::ecs::Without<r::GlobalTransform3d>>;

using OrphelinsQuery = r::ecs::Query<
    r::ecs::Without<r::ecs::Parent>>;

using TransformComponentsQuery = r::ecs::Query<
    r::ecs::Ref<r::Transform3d>,
    r::ecs::Mut<r::GlobalTransform3d>,
    r::ecs::Optional<r::ecs::Ref<r::ecs::Children>>>;

struct TransformPluginCache {
    public:
        std::unordered_map<r::ecs::Entity, const r::Transform3d *> local;
        std::unordered_map<r::ecs::Entity, r::GlobalTransform3d *> global;
        std::unordered_map<r::ecs::Entity, const std::vector<r::ecs::Entity> *> children;
};

// clang-format on

/**
 * static helpers
 */

/**
 * @brief builds a cache of direct component ptr
 * @param all_transforms_q query containing all transform-related components
 * @return populated TransformPluginCache
 */
TransformPluginCache transform_build_cache(TransformComponentsQuery &all_transforms_q)
{
    TransformPluginCache cache;
    const size_t entity_count = all_transforms_q.size();

    cache.local.reserve(entity_count);
    cache.global.reserve(entity_count);
    cache.children.reserve(entity_count / 4);///<< idk maybe a quarter of entities have children? ((to avoid realloc))

    for (auto it = all_transforms_q.begin(); it != all_transforms_q.end(); ++it) {
        const auto [local, global, children_opt] = *it;
        const r::ecs::Entity entity = it.entity();

        cache.local[entity] = local.ptr;
        cache.global[entity] = global.ptr;

        if (children_opt.ptr && children_opt.ptr->ptr) {
            cache.children[entity] = &children_opt.ptr->ptr->entities;
        }
    }

    return cache;
}

/**
 * @brief initializes root transforms and prepares the queue
 * @param roots_q query to find all root entities
 * @param cache  pre-built transform cache
 * @return deque containing all root entities, ready for traversal.
 */
std::deque<r::ecs::Entity> transform_get_queue(OrphelinsQuery &roots_q, const TransformPluginCache &cache)
{
    std::deque<r::ecs::Entity> queue;

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

    return queue;
}

/**
 * @brief Breadth-First Search to propagate transforms to all descendants
 * @param queue queue, pre-populated with root entities.
 * @param cache pre-built transform cache.
 */
void transform_bfs_propagation(std::deque<r::ecs::Entity> &queue, const TransformPluginCache &cache)
{
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

/**
 * TransformPlugin systems
 */

/**
 * @brief as the name suggests, adds missing GlobalTransform3d components
 */
static void transform_add_missing_global_system(r::ecs::Commands &commands, TransformWithoutGlobal query)
{
    for (auto it = query.begin(); it != query.end(); ++it) {
        commands.entity(it.entity()).insert(r::GlobalTransform3d{});
    }
}

/**
 * @brief high-level system that orchestrates the transform propagation process
 */
static void transform_propagate_system(OrphelinsQuery roots_q, TransformComponentsQuery all_transforms_q)
{
    if (all_transforms_q.size() == 0) {
        return;
    }

    const TransformPluginCache cache = transform_build_cache(all_transforms_q);
    std::deque<r::ecs::Entity> queue = transform_get_queue(roots_q, cache);

    transform_bfs_propagation(queue, cache);
}

}// namespace

void r::TransformPlugin::build(Application &app)
{
    app.add_systems<transform_add_missing_global_system>(r::Schedule::UPDATE)
        .add_systems<transform_propagate_system>(r::Schedule::BEFORE_RENDER_2D);
}
