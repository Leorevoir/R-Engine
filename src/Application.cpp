#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Clock.hpp>
#include <R-Engine/Core/Error.hpp>
#include <R-Engine/Core/Logger.hpp>

#include <csignal>
#include <iostream>
#include <queue>
#include <sstream>

r::Application::SystemNode::SystemNode() : id(typeid(void))
{
}

r::Application::SystemNode::SystemNode(std::string p_name, SystemTypeId p_id,
    std::function<void(ecs::Scene &, ecs::CommandBuffer &)> p_func, std::unordered_set<SystemTypeId> p_dependencies)
    : name(std::move(p_name)), id(p_id), func(std::move(p_func)), dependencies(std::move(p_dependencies))
{
}

/**
* public
*/

r::Application::Application()
{
    Logger::info("Application created");
    std::signal(SIGINT, [](int) {
        std::cout << "\r";
        Logger::info("SIGINT received, quitting application...");
        r::Application::quit = true;
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
    _scene.insert_resource<core::FrameTime>(_clock.frame());

    Logger::debug("Pre-startup schedule running...");
    _run_schedule(Schedule::PRE_STARTUP);
    _apply_commands();

    Logger::debug("Startup schedule running...");
    _run_schedule(Schedule::STARTUP);
    _apply_commands();
    Logger::debug("Startup schedule complete. Entering main loop.");
}

void r::Application::_main_loop()
{
    while (!quit) {
        _clock.tick();
        *_scene.get_resource_ptr<core::FrameTime>() = _clock.frame();

        _run_schedule(Schedule::UPDATE);
        _apply_commands();

        for (i32 i = 0; i < _clock.frame().substep_count; ++i) {
            _run_schedule(Schedule::FIXED_UPDATE);
            _apply_commands();
        }
        _render_routine();
    }
}

void r::Application::_shutdown()
{
    Logger::debug("Main loop exited. Running shutdown schedule...");
    _run_schedule(Schedule::SHUTDOWN);
    _apply_commands();
    Logger::debug("Shutdown schedule complete. Application exiting.");
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
    std::queue<SystemTypeId> q;

    for (const auto &[id, node] : graph.nodes) {
        if (!node.func) {
            throw exception::Error("Scheduler", "System '", node.name, "' was added as a dependency but was never defined.");
        }
        in_degree[id] = 0;
    }

    for (const auto &[id, node] : graph.nodes) {
        for (const auto &dep_id : node.dependencies) {
            if (graph.nodes.find(dep_id) == graph.nodes.end()) {
                throw exception::Error("Scheduler", "System dependency '", dep_id.name(), "' not found for system '", node.name, "'.");
            }
            adj_list[dep_id].push_back(id);
            in_degree[id]++;
        }
    }

    for (const auto &[id, _] : graph.nodes) {
        if (in_degree[id] == 0) {
            q.push(id);
        }
    }

    while (!q.empty()) {
        SystemTypeId u = q.front();
        q.pop();
        graph.execution_order.push_back(u);

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

    graph.dirty = false;
}

void r::Application::_execute_systems(const ScheduleGraph &graph)
{
    for (const auto &system_id : graph.execution_order) {
        graph.nodes.at(system_id).func(_scene, _command_buffer);
    }
}

void r::Application::_render_routine()
{
    _run_schedule(Schedule::BEFORE_RENDER_3D);
    _run_schedule(Schedule::RENDER_3D);
    _run_schedule(Schedule::AFTER_RENDER_3D);
    _run_schedule(Schedule::BEFORE_RENDER_2D);
    _run_schedule(Schedule::RENDER_2D);
    _run_schedule(Schedule::AFTER_RENDER_2D);
}

void r::Application::_apply_commands()
{
    _command_buffer.apply(_scene);
}
