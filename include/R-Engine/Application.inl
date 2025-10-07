#pragma once

#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/Plugin.hpp>
#include <type_traits>
#include <utility>

// clang-format off

/**
* ConfiguratorBase Implementation
*/
template<typename Derived>
template<auto... SystemFuncs>
inline r::Application::SystemConfigurator r::Application::ConfiguratorBase<Derived>::add_systems(Schedule when) noexcept
{
    return _app->add_systems<SystemFuncs...>(when);
}

template<typename Derived>
template<typename... SetTypes>
inline auto r::Application::ConfiguratorBase<Derived>::configure_sets(Schedule when) noexcept -> r::Application::SetConfigurator<SetTypes...>
{
    return _app->configure_sets<SetTypes...>(when);
}

/**
* SystemConfigurator Implementation
*/

inline r::Application::SystemConfigurator::SystemConfigurator(Application *app, Schedule schedule, std::vector<SystemTypeId> system_ids) noexcept
    : ConfiguratorBase(app), _schedule(schedule), _system_ids(std::move(system_ids))
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

template<typename SetType>
inline r::Application::SystemConfigurator &r::Application::SystemConfigurator::after() noexcept
{
    SystemSetId dependency_set_id = _app->template _ensure_set_exists<SetType>(_schedule);
    auto &graph = _app->_systems[_schedule];

    for (const auto &system_id : _system_ids) {
        auto &node = graph.nodes.at(system_id);
        auto &deps = node.after_sets;
        if (std::find(deps.begin(), deps.end(), dependency_set_id) == deps.end()) {
            deps.push_back(dependency_set_id);
        }
    }

    graph.dirty = true;
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

template<typename SetType>
inline r::Application::SystemConfigurator &r::Application::SystemConfigurator::before() noexcept
{
    SystemSetId dependent_set_id = _app->template _ensure_set_exists<SetType>(_schedule);
    auto &graph = _app->_systems[_schedule];

    for (const auto &system_id : _system_ids) {
        auto &node = graph.nodes.at(system_id);
        auto &deps = node.before_sets;
        if (std::find(deps.begin(), deps.end(), dependent_set_id) == deps.end()) {
            deps.push_back(dependent_set_id);
        }
    }

    graph.dirty = true;
    return *this;
}

template<typename SetType>
inline r::Application::SystemConfigurator &r::Application::SystemConfigurator::in_set() noexcept
{
    SystemSetId set_id = _app->template _ensure_set_exists<SetType>(_schedule);
    auto &graph = _app->_systems[_schedule];

    /* Add all current systems to this set */
    for (const auto &system_id : _system_ids) {
        auto &node = graph.nodes.at(system_id);
        if (std::find(node.member_of_sets.begin(), node.member_of_sets.end(), set_id)
            == node.member_of_sets.end()) {
            node.member_of_sets.push_back(set_id);
        }
    }

    graph.dirty = true;
    return *this;
}

/**
* SetConfigurator Implementation
*/

template<typename... SetTypes>
r::Application::SetConfigurator<SetTypes...>::SetConfigurator(
    Application *app, Schedule schedule, std::vector<SystemSetId> setids) noexcept
    : ConfiguratorBase<SetConfigurator<SetTypes...>>(app), _schedule(schedule), _set_ids(std::move(setids))
{
    /* __ctor__ */
}

template<typename... SetTypes>
template<typename OtherSet>
inline auto r::Application::SetConfigurator<SetTypes...>::before() noexcept
    -> SetConfigurator&
{
    SystemSetId other_set_id = this->_app->template _ensure_set_exists<OtherSet>(_schedule);
    auto &graph = this->_app->_systems[_schedule];

    /* Add before constraint to all configured sets */
    for (const auto &set_id : _set_ids) {
        auto &set = graph.sets.at(set_id);
        if (std::find(set.before_sets.begin(), set.before_sets.end(), other_set_id)
            == set.before_sets.end()) {
            set.before_sets.push_back(other_set_id);
        }
    }

    graph.dirty = true;
    return *this;
}

template<typename... SetTypes>
template<typename OtherSet>
inline auto r::Application::SetConfigurator<SetTypes...>::after() noexcept
    -> SetConfigurator&
{
    SystemSetId other_set_id = this->_app->template _ensure_set_exists<OtherSet>(_schedule);
    auto &graph = this->_app->_systems[_schedule];

    /* Add after constraint to all configured sets */
    for (const auto &set_id : _set_ids) {
        auto &other_set = graph.sets.at(other_set_id);
        if (std::find(other_set.before_sets.begin(), other_set.before_sets.end(), set_id)
            == other_set.before_sets.end()) {
            other_set.before_sets.push_back(set_id);
        }
    }

    graph.dirty = true;
    return *this;
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
        node.member_of_sets = std::move(graph.nodes.at(id).member_of_sets);
    }

    graph.nodes.insert_or_assign(id, std::move(node));
    graph.dirty = true;

    return id;
}

template<typename SetType>
r::Application::SystemSetId r::Application::_ensure_set_exists(Schedule when) noexcept
{
    SystemSetId id = typeid(SystemSetTag<SetType>);
    auto &graph = _systems[when];

    if (graph.sets.find(id) == graph.sets.end()) {
        graph.sets.emplace(id, SystemSet(typeid(SetType).name(), id));
    }

    return id;
}

template<auto... SystemFuncs>
r::Application::SystemConfigurator r::Application::add_systems(Schedule when) noexcept
{
    std::vector<SystemTypeId> ids = {_add_one_system<SystemFuncs>(when)...};

    return SystemConfigurator(this, when, std::move(ids));
}

template<typename... SetTypes>
inline auto r::Application::configure_sets(Schedule when) noexcept
    -> SetConfigurator<SetTypes...>
{
    std::vector<SystemSetId> ids = {(_ensure_set_exists<SetTypes>(when))...};
    return SetConfigurator<SetTypes...>(this, when, std::move(ids));
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

namespace r {

namespace detail {

/**
* @brief system to clear events after they have been processed
*/
template<typename EventT>
static void __clear_events_system(ecs::ResMut<ecs::Events<EventT>> events)
{
    if (events.ptr) {
        events.ptr->clear();
    }
}

}// namespace detail

}// namespace r

template<typename... EventTs>
r::Application &r::Application::add_events() noexcept
{
    (insert_resource(ecs::Events<EventTs>{}), ...);
    (add_systems<detail::__clear_events_system<EventTs>>(Schedule::EVENT_CLEANUP), ...);

    return *this;
}
