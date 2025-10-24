#pragma once

#include "R-Engine/Types.hpp"
#include <R-Engine/R-EngineExport.hpp>

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace r {

class Application;

namespace ecs {

class Scene;
class CommandBuffer;

}// namespace ecs

namespace sys {

struct ScheduleGraph;
struct SystemNode;
using SystemTypeId = std::type_index;

}// namespace sys
//

namespace core {

class ThreadPool;

// clang-format off

/**
 * @brief Manages the sorting and execution of systems within a ScheduleGraph.
 * @details This class encapsulates the logic for topological sorting of system dependencies
 * and executing the resulting stages, utilizing a thread pool for parallel systems.
 */
class R_ENGINE_API Scheduler final
{
    public:
        explicit Scheduler(ThreadPool &thread_pool);
        ~Scheduler() = default;

        /**
         * @brief Runs all systems in a given schedule graph.
         * @details If the graph is marked as 'dirty', it will be re-sorted before execution.
         */
        void run(
            sys::ScheduleGraph &graph,
            ecs::Scene &scene,
            ecs::CommandBuffer &main_command_buffer,
            const std::vector<std::unique_ptr<ecs::CommandBuffer>> &thread_local_buffers
        );

    private:
        void _sort_graph(sys::ScheduleGraph &graph);
        void _execute_graph(
            const sys::ScheduleGraph &graph,
            ecs::Scene &scene,
            ecs::CommandBuffer &main_command_buffer,
            const std::vector<std::unique_ptr<ecs::CommandBuffer>> &thread_local_buffers
        );

        void _build_adjacency_list(
            const sys::ScheduleGraph &graph,
            std::unordered_map<sys::SystemTypeId, i32> &in_degree,
            std::unordered_map<sys::SystemTypeId, std::vector<sys::SystemTypeId>> &adj_list
        );

        void _apply_set_ordering_constraints(
            const sys::ScheduleGraph &graph,
            std::unordered_map<sys::SystemTypeId, i32> &in_degree,
            std::unordered_map<sys::SystemTypeId, std::vector<sys::SystemTypeId>> &adj_list
        );

        std::vector<const sys::SystemNode *> _select_systems_for_stage(
            const std::vector<sys::SystemTypeId> &ready_systems,
            const sys::ScheduleGraph &graph
        );

        ThreadPool &_thread_pool;
};

}// namespace core

}// namespace r
