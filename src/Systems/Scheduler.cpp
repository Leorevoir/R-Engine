#include <R-Engine/Systems/ScheduleGraph.hpp>
#include <R-Engine/Systems/Scheduler.hpp>

#include <R-Engine/Core/Error.hpp>
#include <R-Engine/Core/ThreadPool.hpp>

#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Scene.hpp>

#include <atomic>
#include <future>
#include <unordered_set>
#include <vector>

// clang-format off

/**
 * static private helpers
 */

namespace {

/**
 * select system for stage helpers
 */

/**
 * @brief check if a system has access conflicts (read/write) with the accesses already present in a stage.
 */
static bool scheduler_system_access_conflict(
    const r::sys::SystemNode &node,
    const r::sys::Access &stage_component_access,
    const r::sys::Access &stage_resource_access
)
{
    for (const auto &write : node.component_access.writes) {
        if (stage_component_access.reads.count(write) || stage_component_access.writes.count(write)) {
            return true;
        }
    }
    for (const auto &read : node.component_access.reads) {
        if (stage_component_access.writes.count(read)) {
            return true;
        }
    }
    for (const auto &write : node.resource_access.writes) {
        if (stage_resource_access.reads.count(write) || stage_resource_access.writes.count(write)) {
            return true;
        }
    }
    for (const auto &read : node.resource_access.reads) {
        if (stage_resource_access.writes.count(read)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief try to form a main thread stage containing a single "main thread only" system.
*/
static std::vector<const r::sys::SystemNode *> scheduler_system_select_main_thread_stage(
    const std::vector<r::sys::SystemTypeId> &ready_systems,
    const r::sys::ScheduleGraph &graph
)
{
    for (const auto &id : ready_systems) {
        const auto &node = graph.nodes.at(id);

        if (node.is_main_thread_only) {
            return {&node}; ///<< a main-thread countains only ONE system
        }
    }
    return {};
}

/**
 @brief Forms an execution stage by aggressively adding all compatible parallel systems.
 */
static std::vector<const r::sys::SystemNode *> scheduler_system_select_parallel_stage(
    const std::vector<r::sys::SystemTypeId> &ready_systems,
    const r::sys::ScheduleGraph &graph
)
{
    std::vector<const r::sys::SystemNode *> stage_nodes;
    r::sys::Access stage_component_access;
    r::sys::Access stage_resource_access;

    for (const auto &id : ready_systems) {
        const auto &node = graph.nodes.at(id);

        /**
         * @info if no conflict -> add system to stage
         */
        if (!scheduler_system_access_conflict(node, stage_component_access, stage_resource_access)) {
            stage_nodes.push_back(&node);
            stage_component_access.reads.insert(node.component_access.reads.begin(), node.component_access.reads.end());
            stage_component_access.writes.insert(node.component_access.writes.begin(), node.component_access.writes.end());
            stage_resource_access.reads.insert(node.resource_access.reads.begin(), node.resource_access.reads.end());
            stage_resource_access.writes.insert(node.resource_access.writes.begin(), node.resource_access.writes.end());
        }
    }
    return stage_nodes;
}

/**
 * execute graph helpers
 */

/**
* @brief execute a single system stage on the main thread.
 */
static void scheduler_system_execute_main_thread_stage(
    const r::sys::SystemNode *node,
    r::ecs::Scene &scene, r::ecs::CommandBuffer &main_command_buffer)
{
    if (!node->condition || node->condition(scene)) {
        node->func(scene, main_command_buffer);
    }
}

/**
 * @brief execute a parallel stage helper
 */
static void scheduler_system_execute_parallel_stage(
    const std::vector<const r::sys::SystemNode *> &stage,
    r::core::ThreadPool &thread_pool,
    r::ecs::Scene &scene,
    const std::vector<std::unique_ptr<r::ecs::CommandBuffer>> &thread_local_buffers
)
{
    thread_local size_t thread_idx = 0;
    static std::atomic<size_t> next_thread_idx{1};

    std::vector<std::future<void>> futures;
    futures.reserve(stage.size());

    for (const auto *node_ptr : stage) {
        futures.emplace_back(thread_pool.enqueue([&, node_ptr] {
            if (thread_idx == 0) {
                thread_idx = next_thread_idx.fetch_add(1);
            }
            if (!node_ptr->condition || node_ptr->condition(scene)) {
                node_ptr->func(scene, *thread_local_buffers[thread_idx % thread_local_buffers.size()]);
            }
        }));
    }

    for (auto &future : futures) {
        future.get();
    }
}

}// namespace

/**
 * public
 */

r::core::Scheduler::Scheduler(core::ThreadPool &thread_pool) : _thread_pool(thread_pool)
{
    /* __ctor__ */
}

void r::core::Scheduler::run(
    sys::ScheduleGraph &graph,
    ecs::Scene &scene,
    ecs::CommandBuffer &main_command_buffer,
    const std::vector<std::unique_ptr<ecs::CommandBuffer>> &thread_local_buffers
)
{
    if (graph.dirty) {
        _sort_graph(graph);
    }
    _execute_graph(graph, scene, main_command_buffer, thread_local_buffers);
}

/**
* private
*/

void r::core::Scheduler::_sort_graph(sys::ScheduleGraph &graph)
{
    graph.execution_stages.clear();

    std::unordered_map<sys::SystemTypeId, int> in_degree;
    std::unordered_map<sys::SystemTypeId, std::vector<sys::SystemTypeId>> adj_list;
    std::unordered_set<sys::SystemTypeId> remaining_systems;

    /**
    * @info data structure initialization
    */
    for (const auto &[id, node] : graph.nodes) {
        if (!node.func) {
            throw exception::Error("Scheduler", "System '", node.name, "' was added as a dependency but was never defined.");
        }
        in_degree[id] = 0;
        remaining_systems.insert(id);
    }

    _build_adjacency_list(graph, in_degree, adj_list);
    _apply_set_ordering_constraints(graph, in_degree, adj_list);

    /**
    * @info topological sort loop
    */
    while (!remaining_systems.empty()) {
        std::vector<sys::SystemTypeId> ready_systems;
        for (const auto &id : remaining_systems) {
            if (in_degree.at(id) == 0) {
                ready_systems.push_back(id);
            }
        }

        if (ready_systems.empty()) {
            throw exception::Error("Scheduler", "Cycle detected in system dependencies.");
        }

        const auto systems_for_stage = _select_systems_for_stage(ready_systems, graph);
        if (systems_for_stage.empty()) {
            throw exception::Error("Scheduler", "Could not schedule any systems, check for dependency cycles.");
        }

        /**
        * @info update dependencies for next iteration
        */
        for (const auto *node : systems_for_stage) {
            remaining_systems.erase(node->id);
            if (adj_list.count(node->id)) {
                for (const auto &dependent : adj_list.at(node->id)) {
                    in_degree.at(dependent)--;
                }
            }
        }
        graph.execution_stages.push_back(std::move(systems_for_stage));
    }
    graph.dirty = false;
}

std::vector<const r::sys::SystemNode *> r::core::Scheduler::_select_systems_for_stage(
    const std::vector<sys::SystemTypeId> &ready_systems,
    const sys::ScheduleGraph &graph
)
{
    auto main_thread_stage = scheduler_system_select_main_thread_stage(ready_systems, graph);

    if (!main_thread_stage.empty()) {
        return main_thread_stage;
    }
    return scheduler_system_select_parallel_stage(ready_systems, graph);
}

void r::core::Scheduler::_execute_graph(
    const sys::ScheduleGraph &graph,
    ecs::Scene &scene,
    ecs::CommandBuffer &main_command_buffer,
    const std::vector<std::unique_ptr<ecs::CommandBuffer>> &thread_local_buffers
)
{
    for (const auto &stage : graph.execution_stages) {

        if (stage.empty()) {
            continue;
        }

        const bool is_main_thread_stage = stage.size() == 1 && stage[0]->is_main_thread_only;

        if (is_main_thread_stage) {
            scheduler_system_execute_main_thread_stage(stage[0], scene, main_command_buffer);
        } else {
            scheduler_system_execute_parallel_stage(stage, _thread_pool, scene, thread_local_buffers);
        }
    }
}

void r::core::Scheduler::_build_adjacency_list(
    const sys::ScheduleGraph &graph,
    std::unordered_map<sys::SystemTypeId, i32> &in_degree,
    std::unordered_map<sys::SystemTypeId, std::vector<sys::SystemTypeId>> &adj_list
)
{
    for (const auto &[id, node] : graph.nodes) {

        for (const auto &dep_id : node.dependencies) {

            if (graph.nodes.find(dep_id) == graph.nodes.end()) {
                throw exception::Error("Scheduler", "System dependency '", dep_id.name(), "' not found for system '", node.name, "'.");
            }

            adj_list[dep_id].push_back(id);
            ++in_degree[id];
        }
    }

}

void r::core::Scheduler::_apply_set_ordering_constraints(
    const sys::ScheduleGraph &graph,
    std::unordered_map<sys::SystemTypeId, i32> &in_degree,
    std::unordered_map<sys::SystemTypeId, std::vector<sys::SystemTypeId>> &adj_list
)
{
    /**
     * @info add dependencies from set ordering and system-to-set ordering
     */
    for (const auto &[id, node] : graph.nodes) {

        /**
        * @info set --> set dependencies
        */
        for (const auto &set_id : node.member_of_sets) {
            if (graph.sets.find(set_id) == graph.sets.end())
                continue;
            const auto &set = graph.sets.at(set_id);
            for (const auto &before_set_id : set.before_sets) {
                if (graph.sets.find(before_set_id) == graph.sets.end())
                    continue;
                for (const auto &[other_id, other_node] : graph.nodes) {
                    if (std::find(other_node.member_of_sets.begin(), other_node.member_of_sets.end(), before_set_id)
                        != other_node.member_of_sets.end()) {
                        auto &neighbors = adj_list[id];
                        if (std::find(neighbors.begin(), neighbors.end(), other_id) == neighbors.end()) {
                            neighbors.push_back(other_id);
                            ++in_degree[other_id];
                        }
                    }
                }
            }
        }

        /**
        * @info system --> set dependencies
        */
        for (const auto &before_set_id : node.before_sets) {
            for (const auto &[other_id, other_node] : graph.nodes) {
                if (std::find(other_node.member_of_sets.begin(), other_node.member_of_sets.end(), before_set_id)
                    != other_node.member_of_sets.end()) {
                    auto &neighbors = adj_list[id];
                    if (std::find(neighbors.begin(), neighbors.end(), other_id) == neighbors.end()) {
                        neighbors.push_back(other_id);
                        ++in_degree[other_id];
                    }
                }
            }
        }

        /**
        * @info set --> system dependencies
        */
        for (const auto &after_set_id : node.after_sets) {
            for (const auto &[other_id, other_node] : graph.nodes) {
                if (std::find(other_node.member_of_sets.begin(), other_node.member_of_sets.end(), after_set_id)
                    != other_node.member_of_sets.end()) {
                    auto &neighbors = adj_list[other_id];
                    if (std::find(neighbors.begin(), neighbors.end(), id) == neighbors.end()) {
                        neighbors.push_back(id);
                        in_degree[id]++;
                    }
                }
            }
        }
    }
}
