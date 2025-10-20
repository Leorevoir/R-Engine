#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Clock.hpp>
#include <R-Engine/Core/Error.hpp>
#include <R-Engine/Core/Logger.hpp>

#include <csignal>
#include <iostream>
#include <queue>
#include <sstream>

r::Application::SystemNode::SystemNode() : id(typeid(void)), func(nullptr)
{
    /* __ctor__ */
}

r::Application::SystemNode::SystemNode(const std::string &p_name, SystemTypeId p_id, SystemFn p_func,
    std::vector<SystemTypeId> p_dependencies)
    : name(std::move(p_name)), id(p_id), func(p_func), dependencies(std::move(p_dependencies))
{
    /* __ctor__ */
}

r::Application::SystemSet::SystemSet(const std::string &pname, SystemSetId pid) noexcept : name(pname), id(pid)
{
    /* __ctor__ */
}

/**
* public
*/

r::Application::Application()
{
    Logger::debug("Application created");
    std::signal(SIGINT, [](i32) {
        r::Application::quit.store(true, std::memory_order_relaxed);
        std::cout << "\r";
        Logger::warn("SIGINT received, quitting application...");
    });
}

void r::Application::run()
{
    _startup();
    _main_loop();
    _shutdown();
}

/**
* private
*/

void r::Application::_startup()
{
    _scene.insert_resource(_clock.frame());

    Logger::debug("Pre-startup schedule running...");
    _run_schedule(Schedule::PRE_STARTUP);
    _apply_commands();

    _systems.erase(Schedule::PRE_STARTUP);
    if (_systems.empty()) {
        quit.store(true, std::memory_order_relaxed);
        return;
    }

    Logger::debug("Startup schedule running...");
    _run_schedule(Schedule::STARTUP);
    _apply_commands();
    Logger::debug("Startup schedule complete. Entering main loop.");
    _systems.erase(Schedule::STARTUP);
    if (_systems.empty()) {
        quit.store(true, std::memory_order_relaxed);
        return;
    }
}

void r::Application::_main_loop()
{
    while (!quit) {
        _clock.tick();
        *_scene.get_resource_ptr<core::FrameTime>() = _clock.frame();

        _apply_state_transitions();

        _run_schedule(Schedule::UPDATE);
        _apply_commands();

        for (i32 i = 0; i < _clock.frame().substep_count; ++i) {
            _run_schedule(Schedule::FIXED_UPDATE);
            _apply_commands();
        }
        _run_schedule(Schedule::EVENT_CLEANUP);
        _render_routine();
    }
}

void r::Application::_shutdown()
{
    Logger::debug("Main loop exited. Running shutdown schedule...");
    _run_schedule(Schedule::SHUTDOWN);
    _apply_commands();
    Logger::debug("Shutdown schedule complete. Application exiting.");
    Logger::debug("Quiting the application...");
}

/**
* private
*/

void r::Application::_run_schedule(const Schedule sched)
{
    const auto it = _systems.find(sched);

    if (it == _systems.end() || it->second.nodes.empty()) {
        return;
    }

    auto &graph = it->second;

    if (graph.dirty) {
        _sort_schedule(graph);
    }

    _execute_systems(graph);
}

void r::Application::_sort_schedule(ScheduleGraph &graph)
{
    graph.execution_order.clear();
    std::unordered_map<SystemTypeId, int> in_degree;
    std::unordered_map<SystemTypeId, std::vector<SystemTypeId>> adj_list;

    /* Initialize in-degree for all systems */
    for (const auto &[id, node] : graph.nodes) {
        if (!node.func) {
            throw exception::Error("Scheduler", "System '", node.name, "' was added as a dependency but was never defined.");
        }
        in_degree[id] = 0;
    }

    _build_adjacency_list(graph, in_degree, adj_list);
    _apply_set_ordering_constraints(graph, in_degree, adj_list);
    _perform_topological_sort(graph, in_degree, adj_list);

    graph.dirty = false;
}

void r::Application::_build_adjacency_list(const ScheduleGraph &graph, std::unordered_map<SystemTypeId, int> &in_degree,
    std::unordered_map<SystemTypeId, std::vector<SystemTypeId>> &adj_list)
{
    /* Build adjacency list from direct system dependencies */
    for (const auto &[id, node] : graph.nodes) {
        for (const auto &dep_id : node.dependencies) {
            if (graph.nodes.find(dep_id) == graph.nodes.end()) {
                throw exception::Error("Scheduler", "System dependency '", dep_id.name(), "' not found for system '", node.name, "'.");
            }
            adj_list[dep_id].push_back(id);
            in_degree[id]++;
        }
    }
}

void r::Application::_apply_set_ordering_constraints(const ScheduleGraph &graph, std::unordered_map<SystemTypeId, int> &in_degree,
    std::unordered_map<SystemTypeId, std::vector<SystemTypeId>> &adj_list)
{
    /* Add dependencies from set ordering and system-to-set ordering */
    for (const auto &[id, node] : graph.nodes) {
        /* 1. Set -> Set dependencies */
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
                            in_degree[other_id]++;
                        }
                    }
                }
            }
        }

        /* 2. System -> Set dependencies */
        for (const auto &before_set_id : node.before_sets) {
            for (const auto &[other_id, other_node] : graph.nodes) {
                if (std::find(other_node.member_of_sets.begin(), other_node.member_of_sets.end(), before_set_id)
                    != other_node.member_of_sets.end()) {
                    auto &neighbors = adj_list[id];
                    if (std::find(neighbors.begin(), neighbors.end(), other_id) == neighbors.end()) {
                        neighbors.push_back(other_id);
                        in_degree[other_id]++;
                    }
                }
            }
        }

        /* 3. Set -> System dependencies */
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

void r::Application::_perform_topological_sort(ScheduleGraph &graph, std::unordered_map<SystemTypeId, int> &in_degree,
    const std::unordered_map<SystemTypeId, std::vector<SystemTypeId>> &adj_list)
{
    /* Topological sort (Kahn's algorithm) */
    std::queue<SystemTypeId> q;
    for (const auto &[id, _] : graph.nodes) {
        if (in_degree[id] == 0) {
            q.push(id);
        }
    }

    while (!q.empty()) {
        SystemTypeId u = q.front();
        q.pop();

        /* push pointer to the node into execution_order */
        graph.execution_order.push_back(&graph.nodes.at(u));

        if (adj_list.count(u)) {
            for (const auto &v : adj_list.at(u)) {
                if (--in_degree[v] == 0) {
                    q.push(v);
                }
            }
        }
    }

    if (graph.execution_order.size() != graph.nodes.size()) {
        std::stringstream ss;
        ss << "Cycle detected in schedule. Executed " << graph.execution_order.size() << " of " << graph.nodes.size()
           << " systems. Remaining systems: \n";
        for (const auto &[id, degree] : in_degree) {
            if (degree > 0) {
                ss << " - " << graph.nodes.at(id).name << " (in-degree: " << degree << ")\n";
            }
        }
        throw exception::Error("Scheduler", ss.str());
    }
}

void r::Application::_execute_systems(const ScheduleGraph &graph)
{
    for (const auto *node_ptr : graph.execution_order) {
        if (node_ptr->condition && !node_ptr->condition(_scene)) {
            continue;
        }
        node_ptr->func(_scene, _command_buffer);
    }
}

void r::Application::_render_routine()
{
    _run_schedule(Schedule::BEFORE_RENDER_2D);
    _run_schedule(Schedule::BEFORE_RENDER_3D);

    _run_schedule(Schedule::RENDER_3D);

    _run_schedule(Schedule::AFTER_RENDER_3D);
    _run_schedule(Schedule::RENDER_2D);

    _run_schedule(Schedule::AFTER_RENDER_2D);
}

void r::Application::_apply_commands()
{
    _command_buffer.apply(_scene);
}

void r::Application::_apply_state_transitions()
{
    if (_state_transition_runner) {
        _state_transition_runner();
    }
}

void r::Application::_run_transition_schedule(ScheduleGraph &graph)
{
    if (graph.nodes.empty())
        return;

    if (graph.dirty) {
        _sort_schedule(graph);
    }
    _execute_systems(graph);
}
