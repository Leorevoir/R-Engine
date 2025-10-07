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

#include <iostream>

template<typename... EventTs>
r::Application &r::Application::add_events() noexcept
{
    std::cout << "add events: " << (typeid(EventTs).name() + ... + std::string(", ")) << std::endl;
    return *this;
}
