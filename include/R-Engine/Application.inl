#pragma once

#include "R-Engine/Application.hpp"
#include <R-Engine/Plugins/Plugin.hpp>

#include <type_traits>
#include <utility>

// clang-format off

/**
* SystemConfigurator Implementation
*/

inline r::Application::SystemConfigurator::SystemConfigurator(Application *app, Schedule schedule, std::vector<SystemTypeId> system_ids) noexcept
    : _app(app), _schedule(schedule), _system_ids(std::move(system_ids))
{
    /* __ctor__ */
}

template<auto SystemFunc>
inline r::Application::SystemConfigurator &r::Application::SystemConfigurator::after() noexcept
{
    SystemTypeId dependency_id(typeid(SystemTag<SystemFunc>));

    for (const auto &system_id : _system_ids) {
        auto &deps = _app->_systems[_schedule].nodes.at(system_id).dependencies;

        if (std::find(deps.begin(), deps.end(), dependency_id) == deps.end()) {
            deps.push_back(dependency_id);
        }

    }
    _app->_systems[_schedule].dirty = true;
    return *this;
}

template<auto SystemFunc>
inline r::Application::SystemConfigurator &r::Application::SystemConfigurator::before() noexcept
{
    SystemTypeId dependent_id(typeid(SystemTag<SystemFunc>));
    auto &graph = _app->_systems[_schedule];

    if (graph.nodes.find(dependent_id) == graph.nodes.end()) {
         const SystemNode placeholder_node(dependent_id.name(), dependent_id, nullptr, {});

         graph.nodes.emplace(dependent_id, std::move(placeholder_node));
    }

    auto &deps = graph.nodes.at(dependent_id).dependencies;

    for (const auto &system_id : _system_ids) {
        if (std::find(deps.begin(), deps.end(), system_id) == deps.end()) {
            deps.push_back(system_id);
        }
    }
    _app->_systems[_schedule].dirty = true;
    return *this;
}

template<auto... SystemFuncs>
inline r::Application::SystemConfigurator r::Application::SystemConfigurator::add_systems(Schedule when) noexcept
{
    return _app->add_systems<SystemFuncs...>(when);
}

template<auto... Funcs, typename StateEnum>
inline r::Application::SystemConfigurator r::Application::SystemConfigurator::add_systems(StateCondition<StateEnum> condition) noexcept
{
    return _app->add_systems<Funcs...>(condition);
}

template<typename ResT>
inline r::Application& r::Application::SystemConfigurator::insert_resource(ResT res) noexcept
{
    return _app->insert_resource(std::move(res));
}

template<typename... Plugins>
inline r::Application& r::Application::SystemConfigurator::add_plugins(Plugins &&...plugins) noexcept
{
    return _app->add_plugins(std::forward<Plugins>(plugins)...);
}

inline void r::Application::SystemConfigurator::run()
{
    _app->run();
}


/**
* Application Implementation
*/

template<auto SystemFunc>
static void system_invoker_template(r::ecs::Scene &scene, r::ecs::CommandBuffer &cmd)
{
    r::ecs::run_system(SystemFunc, scene, cmd);
}

template<auto SystemFunc>
r::Application::SystemTypeId r::Application::_add_one_system(r::Schedule when) noexcept
{
    SystemTypeId id(typeid(SystemTag<SystemFunc>));
    auto &graph = _systems[when];

    SystemNode node(
        id.name(),
        id,
        &system_invoker_template<SystemFunc>,
        {}
    );

    if (graph.nodes.count(id)) {
        node.dependencies = std::move(graph.nodes.at(id).dependencies);
    }

    graph.nodes.insert_or_assign(id, std::move(node));
    graph.dirty = true;

    return id;
}


template<auto... SystemFuncs>
r::Application::SystemConfigurator r::Application::add_systems(Schedule when) noexcept
{
    std::vector<SystemTypeId> ids = {_add_one_system<SystemFuncs>(when)...};

    return SystemConfigurator(this, when, std::move(ids));
}

template<typename ResT>
r::Application &r::Application::insert_resource(ResT res) noexcept
{
    _scene.insert_resource<ResT>(std::move(res));
    return *this;
}
// clang-format on

template<typename PluginT>
void r::Application::_add_one_plugin(PluginT &&plugin) noexcept
{
    using DecayedPluginT = std::decay_t<PluginT>;

    if constexpr (std::is_base_of_v<Plugin, DecayedPluginT> || std::is_base_of_v<PluginGroup, DecayedPluginT>) {
        plugin.build(*this);
    } else {
        static_assert(always_false<PluginT>, "Arguments to add_plugins must be instances of r::Plugin or r::PluginGroup.");
    }
}

template<typename... Plugins>
r::Application &r::Application::add_plugins(Plugins &&...plugins) noexcept
{
    (_add_one_plugin(std::forward<Plugins>(plugins)), ...);
    return *this;
}

template<auto SystemFunc>
r::Application::SystemTypeId r::Application::_add_one_system_to_graph(ScheduleGraph& graph) noexcept
{
    SystemTypeId id(typeid(SystemTag<SystemFunc>));
    
    SystemNode node(
        id.name(),
        id,
        &system_invoker_template<SystemFunc>,
        {}
    );

    if (graph.nodes.count(id)) {
        node.dependencies = std::move(graph.nodes.at(id).dependencies);
    }

    graph.nodes.insert_or_assign(id, std::move(node));
    graph.dirty = true;
    return id;
}


template<typename T>
r::Application& r::Application::init_state(T initial_state) noexcept
{
    insert_resource(State<T>(initial_state));
    insert_resource(NextState<T>());

    _state_transition_runner = [this]() {
        auto* next_state_res = _scene.get_resource_ptr<NextState<T>>();
        if (!next_state_res || !next_state_res->next.has_value()) {
            return;
        }

        auto* state_res = _scene.get_resource_ptr<State<T>>();
        T next = next_state_res->next.value();
        T current = state_res->current();

        if (current == next) {
            next_state_res->next.reset();
            return;
        }

        auto& schedules = _states[typeid(T)];

        // 1. Execute OnExit
        _run_transition_schedule(schedules.on_exit);

        // 2. Execute OnTransition
        if (auto it = schedules.on_transition.find(static_cast<size_t>(current)); it != schedules.on_transition.end()) {
            _run_transition_schedule(it->second);
        }
        
        _apply_commands();

        // 3.Update the state
        state_res->_previous = current;
        state_res->_current = next;

        // 4. Execute OnEnter
        _run_transition_schedule(schedules.on_enter);

        _apply_commands();

        next_state_res->next.reset();
    };

    return *this;
}

// add_systems(OnEnter(...), ...)
template<auto... Funcs, typename StateEnum>
r::Application::SystemConfigurator r::Application::add_systems(StateCondition<StateEnum> condition)
{
    auto& state_schedules = _states[typeid(StateEnum)];
    ScheduleGraph* target_graph = nullptr;
    Schedule schedule_for_configurator = Schedule::UPDATE;

    switch (condition.trigger) {
        case StateTrigger::OnEnter: target_graph = &state_schedules.on_enter; break;
        case StateTrigger::OnExit: target_graph = &state_schedules.on_exit; break;
        case StateTrigger::OnTransition:
            target_graph = &state_schedules.on_transition[static_cast<size_t>(condition.state_from.value())];
            break;
    }

    if (!target_graph) {
        throw exception::Error("Application", "Condition d'état invalide.");
    }

    std::vector<SystemTypeId> ids;
    (ids.push_back(_add_one_system_to_graph<Funcs>(*target_graph)), ...);
    
    // NOTE : Pour simplifier, le SystemConfigurator pour les transitions
    // pourrait avoir des limitations (pas de .before/.after entre schedules différents).
    // Ici, nous le faisons fonctionner pour les dépendances au sein du même graphe de transition.
    return SystemConfigurator(this, schedule_for_configurator, std::move(ids));
}