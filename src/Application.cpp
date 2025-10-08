#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Clock.hpp>
#include <R-Engine/Core/Error.hpp>
#include <R-Engine/Core/Logger.hpp>

#include <csignal>
#include <iostream>
#include <queue>
#include <sstream>

namespace r {

// ScheduleId Implementation
ScheduleId::ScheduleId(Schedule sched)
    : type(Type::FIXED), fixed_schedule(sched), state_type(std::nullopt), state_value(std::nullopt), from_state_value(std::nullopt)
{
}

ScheduleId::ScheduleId(Type t, std::type_index state_t, size_t state_val)
    : type(t), fixed_schedule(std::nullopt), state_type(state_t), state_value(state_val), from_state_value(std::nullopt)
{
}

ScheduleId::ScheduleId(Type t, std::type_index state_t, size_t to_val, size_t from_val)
    : type(t), fixed_schedule(std::nullopt), state_type(state_t), state_value(to_val), from_state_value(from_val)
{
}

bool ScheduleId::operator==(const ScheduleId &other) const
{
    if (type != other.type)
        return false;
    if (fixed_schedule != other.fixed_schedule)
        return false;
    if (state_type != other.state_type)
        return false;
    if (state_value != other.state_value)
        return false;
    if (from_state_value != other.from_state_value)
        return false;
    return true;
}

}// namespace r

// Hash implementation
std::size_t std::hash<r::ScheduleId>::operator()(const r::ScheduleId &id) const noexcept
{
    std::size_t h = 0;
    h ^= std::hash<int>{}(static_cast<int>(id.type)) + 0x9e3779b9 + (h << 6) + (h >> 2);

    if (id.fixed_schedule.has_value()) {
        h ^= std::hash<int>{}(static_cast<int>(id.fixed_schedule.value())) + 0x9e3779b9 + (h << 6) + (h >> 2);
    }

    if (id.state_type.has_value()) {
        h ^= id.state_type.value().hash_code() + 0x9e3779b9 + (h << 6) + (h >> 2);
    }

    if (id.state_value.has_value()) {
        h ^= std::hash<size_t>{}(id.state_value.value()) + 0x9e3779b9 + (h << 6) + (h >> 2);
    }

    if (id.from_state_value.has_value()) {
        h ^= std::hash<size_t>{}(id.from_state_value.value()) + 0x9e3779b9 + (h << 6) + (h >> 2);
    }

    return h;
}

namespace r {

Application::SystemNode::SystemNode() : id(typeid(void)), func(nullptr)
{
    /* __ctor__ */
}

Application::SystemNode::SystemNode(const std::string &p_name, SystemTypeId p_id, SystemFn p_func, std::vector<SystemTypeId> p_dependencies)
    : name(std::move(p_name)), id(p_id), func(p_func), dependencies(std::move(p_dependencies))
{
    /* __ctor__ */
}

Application::SystemSet::SystemSet(const std::string &pname, SystemSetId pid) noexcept : name(pname), id(pid)
{
    /* __ctor__ */
}

/**
* public
*/

Application::Application()
{
    Logger::info("Application created");
    std::signal(SIGINT, [](i32) { r::Application::quit.store(true, std::memory_order_relaxed); });
}

void Application::run()
{
    _startup();
    _main_loop();
    _shutdown();
}

/**
* private
*/

void Application::_startup()
{
    _scene.insert_resource<core::FrameTime>(_clock.frame());

    Logger::debug("Pre-startup schedule running...");
    _run_schedule(ScheduleId(Schedule::PRE_STARTUP));
    _apply_commands();

    _all_schedules.erase(ScheduleId(Schedule::PRE_STARTUP));
    if (_all_schedules.empty()) {
        quit.store(true, std::memory_order_relaxed);
        return;
    }

    Logger::debug("Startup schedule running...");
    _run_schedule(ScheduleId(Schedule::STARTUP));
    _apply_commands();
    Logger::debug("Startup schedule complete. Entering main loop.");
    _all_schedules.erase(ScheduleId(Schedule::STARTUP));
    if (_all_schedules.empty()) {
        quit.store(true, std::memory_order_relaxed);
        return;
    }
}

void Application::_main_loop()
{
    while (!quit) {
        _clock.tick();
        *_scene.get_resource_ptr<core::FrameTime>() = _clock.frame();

        _apply_state_transitions();

        _run_schedule(ScheduleId(Schedule::UPDATE));
        _apply_commands();

        for (i32 i = 0; i < _clock.frame().substep_count; ++i) {
            _run_schedule(ScheduleId(Schedule::FIXED_UPDATE));
            _apply_commands();
        }
        _run_schedule(ScheduleId(Schedule::EVENT_CLEANUP));
        _render_routine();
    }
}

void Application::_shutdown()
{
    Logger::debug("Main loop exited. Running shutdown schedule...");
    _run_schedule(ScheduleId(Schedule::SHUTDOWN));
    _apply_commands();
    if (quit.load(std::memory_order_relaxed)) {
        std::cout << "\r";
        Logger::warn("SIGINT received, quitting application...");
    }
    Logger::debug("Shutdown schedule complete. Application exiting.");
}

/**
* private
*/

void Application::_run_schedule(const ScheduleId &sched)
{
    const auto it = _all_schedules.find(sched);

    if (it == _all_schedules.end() || it->second.nodes.empty()) {
        return;
    }

    auto &graph = it->second;

    if (graph.dirty) {
        _sort_schedule(graph);
    }

    _execute_systems(graph);
}

void Application::_sort_schedule(ScheduleGraph &graph)
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

void Application::_build_adjacency_list(const ScheduleGraph &graph, std::unordered_map<SystemTypeId, int> &in_degree,
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

void Application::_apply_set_ordering_constraints(const ScheduleGraph &graph, std::unordered_map<SystemTypeId, int> &in_degree,
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

void Application::_perform_topological_sort(ScheduleGraph &graph, std::unordered_map<SystemTypeId, int> &in_degree,
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

void Application::_execute_systems(const ScheduleGraph &graph)
{
    for (const auto *node_ptr : graph.execution_order) {
        node_ptr->func(_scene, _command_buffer);
    }
}

void Application::_render_routine()
{
    _run_schedule(ScheduleId(Schedule::BEFORE_RENDER_2D));
    _run_schedule(ScheduleId(Schedule::BEFORE_RENDER_3D));

    _run_schedule(ScheduleId(Schedule::RENDER_3D));

    _run_schedule(ScheduleId(Schedule::AFTER_RENDER_3D));
    _run_schedule(ScheduleId(Schedule::RENDER_2D));

    _run_schedule(ScheduleId(Schedule::AFTER_RENDER_2D));
}

void Application::_apply_commands()
{
    _command_buffer.apply(_scene);
}

void Application::_apply_state_transitions()
{
    if (_state_transition_runner) {
        _state_transition_runner();
    }
}

ScheduleId Application::_build_schedule_id_from_state_condition(StateTrigger trigger, std::type_index state_type, size_t state_value,
    std::optional<size_t> from_value)
{
    switch (trigger) {
        case StateTrigger::OnEnter:
            return ScheduleId(ScheduleId::Type::ON_ENTER, state_type, state_value);
        case StateTrigger::OnExit:
            return ScheduleId(ScheduleId::Type::ON_EXIT, state_type, state_value);
        case StateTrigger::OnTransition:
            if (from_value.has_value()) {
                return ScheduleId(ScheduleId::Type::ON_TRANSITION, state_type, state_value, from_value.value());
            }
            throw exception::Error("Application", "OnTransition requires a from_state value.");
        default:
            throw exception::Error("Application", "Invalid state trigger type.");
    }
}

}// namespace r
