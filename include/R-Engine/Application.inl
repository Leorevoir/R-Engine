#pragma once

#include <R-Engine/Application.hpp>
#include <R-Engine/ECS/System.hpp>
#include <R-Engine/Plugins/Plugin.hpp>
#include <type_traits>
#include <utility>

// clang-format off

namespace r::detail {
    /* Type traits to identify state schedule labels */
    template<typename> struct is_on_enter : std::false_type {};
    template<typename T> struct is_on_enter<OnEnter<T>> : std::true_type {};

    template<typename> struct is_on_exit : std::false_type {};
    template<typename T> struct is_on_exit<OnExit<T>> : std::true_type {};

    template<typename> struct is_on_transition : std::false_type {};
    template<typename T> struct is_on_transition<OnTransition<T>> : std::true_type {};
}

/**
* Application::States Implementation
*/

inline bool r::Application::States::Transition::operator==(const Transition &other) const
{
    return from == other.from && to == other.to;
}

inline usize r::Application::States::TransitionHasher::operator()(const Transition &t) const
{
    usize h1 = std::hash<usize>{}(t.from);
    usize h2 = std::hash<usize>{}(t.to);
    return h1 ^ (h2 << 1);
}

/**
* ConfiguratorBase Implementation
*/

template<typename Derived>
template<auto... SystemFuncs, typename ScheduleLabel>
inline r::Application::SystemConfigurator r::Application::ConfiguratorBase<Derived>::add_systems(ScheduleLabel label) noexcept
{
    return _app->add_systems<SystemFuncs...>(label);
}

template<typename Derived>
template<typename... SetTypes>
inline auto r::Application::ConfiguratorBase<Derived>::configure_sets(Schedule when) noexcept -> r::Application::SetConfigurator<SetTypes...>
{
    return _app->configure_sets<SetTypes...>(when);
}

template<typename Derived>
template<typename ResT>
inline Derived &r::Application::ConfiguratorBase<Derived>::insert_resource(ResT res) noexcept
{
    _app->insert_resource(std::move(res));
    return static_cast<Derived &>(*this);
}

template<typename Derived>
template<typename... Plugins>
inline Derived &r::Application::ConfiguratorBase<Derived>::add_plugins(Plugins &&...plugins) noexcept
{
    _app->add_plugins(std::forward<Plugins>(plugins)...);
    return static_cast<Derived &>(*this);
}

template<typename Derived>
inline void r::Application::ConfiguratorBase<Derived>::run()
{
    _app->run();
}

/**
* SystemConfigurator Implementation
*/

inline r::Application::SystemConfigurator::SystemConfigurator(Application *app, ScheduleGraph *graph, std::vector<SystemTypeId> system_ids) noexcept
    : ConfiguratorBase(app), _graph(graph), _system_ids(std::move(system_ids))
{
    /* __ctor__ */
}

template<auto SystemFunc>
inline r::Application::SystemConfigurator &r::Application::SystemConfigurator::after() noexcept
{
    SystemTypeId dependency_id(typeid(SystemTag<SystemFunc>));

    for (const auto &system_id : _system_ids) {
        auto &deps = _graph->nodes.at(system_id).dependencies;

        if (std::find(deps.begin(), deps.end(), dependency_id) == deps.end()) {
            deps.push_back(dependency_id);
        }

    }
    _graph->dirty = true;
    return *this;
}

template<typename SetType>
inline r::Application::SystemConfigurator &r::Application::SystemConfigurator::after() noexcept
{
    SystemSetId dependency_set_id = _app->_ensure_set_exists<SetType>(*_graph);
    for (const auto &system_id : _system_ids) {
        _graph->nodes.at(system_id).after_sets.push_back(dependency_set_id);
    }
    _graph->dirty = true;
    return *this;
}

template<auto SystemFunc>
inline r::Application::SystemConfigurator &r::Application::SystemConfigurator::before() noexcept
{
    SystemTypeId dependent_id(typeid(SystemTag<SystemFunc>));
    auto &graph = _graph;

    if (graph->nodes.find(dependent_id) == graph->nodes.end()) {
         const SystemNode placeholder_node(dependent_id.name(), dependent_id, nullptr, {});

         graph->nodes.emplace(dependent_id, std::move(placeholder_node));
    }

    auto &deps = graph->nodes.at(dependent_id).dependencies;

    for (const auto &system_id : _system_ids) {
        if (std::find(deps.begin(), deps.end(), system_id) == deps.end()) {
            deps.push_back(system_id);
        }
    }
    _graph->dirty = true;
    return *this;
}

template<typename SetType>
inline r::Application::SystemConfigurator &r::Application::SystemConfigurator::before() noexcept
{
    SystemSetId dependent_set_id = _app->_ensure_set_exists<SetType>(*_graph);
    for (const auto &system_id : _system_ids) {
        _graph->nodes.at(system_id).before_sets.push_back(dependent_set_id);
    }
    _graph->dirty = true;
    return *this;
}

template<auto PredicateFunc>
inline r::Application::SystemConfigurator &r::Application::SystemConfigurator::run_if() noexcept
{
    using traits = ecs::function_traits<std::remove_cvref_t<decltype(PredicateFunc)>>;
    using args = typename traits::args;

    auto condition_wrapper = [app = this->_app](ecs::Scene &scene) -> bool {
        return ecs::call_predicate_with_resolved(
            PredicateFunc, scene, app->_command_buffer, args{}, std::make_index_sequence<std::tuple_size_v<args>>{});
    };

    for (const auto &system_id : _system_ids) {
        _graph->nodes.at(system_id).condition = condition_wrapper;
    }
    return *this;
}

template<typename SetType>
inline r::Application::SystemConfigurator &r::Application::SystemConfigurator::in_set() noexcept
{
    SystemSetId set_id = _app->_ensure_set_exists<SetType>(*_graph);
    for (const auto &system_id : _system_ids) {
        auto &node = _graph->nodes.at(system_id);
        if (std::find(node.member_of_sets.begin(), node.member_of_sets.end(), set_id) == node.member_of_sets.end()) {
            node.member_of_sets.push_back(set_id);
        }
    }
    _graph->dirty = true;
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
    auto& graph = this->_app->_systems[_schedule];
    SystemSetId other_set_id = this->_app->template _ensure_set_exists<OtherSet>(graph);

    for (const auto &set_id : _set_ids) {
        auto &set = graph.sets.at(set_id);
        if (std::find(set.before_sets.begin(), set.before_sets.end(), other_set_id) == set.before_sets.end()) {
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
    auto& graph = this->_app->_systems[_schedule];
    SystemSetId other_set_id = this->_app->template _ensure_set_exists<OtherSet>(graph);

    for (const auto &set_id : _set_ids) {
        auto &other_set = graph.sets.at(other_set_id);
        if (std::find(other_set.before_sets.begin(), other_set.before_sets.end(), set_id) == other_set.before_sets.end()) {
            other_set.before_sets.push_back(set_id); /* Inverted logic for `after` */
        }
    }
    graph.dirty = true;
    return *this;
}

/**
* Application Implementation
*/

template<typename T>
T *r::Application::get_resource_ptr() noexcept
{
    return _scene.get_resource_ptr<T>();
}

template<auto SystemFunc>
static void system_invoker_template(r::ecs::Scene &scene, r::ecs::CommandBuffer &cmd)
{
    r::ecs::run_system(SystemFunc, scene, cmd);
}

template<typename SetType>
r::Application::SystemSetId r::Application::_ensure_set_exists(ScheduleGraph& graph) noexcept
{
    SystemSetId id = typeid(SystemSetTag<SetType>);
    if (graph.sets.find(id) == graph.sets.end()) {
        graph.sets.emplace(id, SystemSet(typeid(SetType).name(), id));
    }
    return id;
}

template<typename... SetTypes>
inline auto r::Application::configure_sets(Schedule when) noexcept
    -> SetConfigurator<SetTypes...>
{
    auto& graph = _systems[when];
    std::vector<SystemSetId> ids = {(_ensure_set_exists<SetTypes>(graph))...};
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
* @brief system to update events after they have been processed
*/
template<typename EventT>
static void __update_events_system(ecs::ResMut<ecs::Events<EventT>> events)
{
    if (events.ptr) {
        events.ptr->update();
    }
}

}// namespace detail

}// namespace r

template<typename... EventTs>
r::Application &r::Application::add_events() noexcept
{
    (insert_resource(ecs::Events<EventTs>{}), ...);
    (add_systems<detail::__update_events_system<EventTs>>(Schedule::EVENT_CLEANUP), ...);

    return *this;
}

template<auto SystemFunc>
r::Application::SystemTypeId r::Application::_add_one_system_to_graph(ScheduleGraph &graph) noexcept
{
    SystemTypeId id(typeid(SystemTag<SystemFunc>));

    SystemNode node(id.name(), id, &system_invoker_template<SystemFunc>, {});

    if (graph.nodes.count(id)) {
        node.dependencies = std::move(graph.nodes.at(id).dependencies);
        node.member_of_sets = std::move(graph.nodes.at(id).member_of_sets);
    }

    graph.nodes.insert_or_assign(id, std::move(node));
    graph.dirty = true;
    return id;
}

template<typename T>
r::Application &r::Application::init_state(T initial_state) noexcept
{
    insert_resource(State<T>(initial_state));
    insert_resource(NextState<T>());

    _state_transition_runner = [this]() {
        auto *state_res = _scene.get_resource_ptr<State<T>>();
        auto *next_state_res = _scene.get_resource_ptr<NextState<T>>();

        /* If no transition is queued, clear the previous state. This ensures `state_changed` is a one-shot condition
        that is only true for the single frame where the transition occurs. */
        if (!next_state_res || !next_state_res->next.has_value()) {
            if (state_res) {
                state_res->_previous.reset();
            }
            return;
        }

        T next = next_state_res->next.value();
        T current = state_res->current();

        if (current == next) {
            next_state_res->next.reset();
            return;
        }

        auto &schedules = _states[typeid(T)];

        /* 1. Execute OnExit for the `current` state */
        if (auto it = schedules.on_exit.find(static_cast<usize>(current)); it != schedules.on_exit.end()) {
            _run_transition_schedule(it->second);
        }

        /* 2. Execute OnTransition */
        typename States::Transition transition{static_cast<usize>(current), static_cast<usize>(next)};
        if (auto it = schedules.on_transition.find(transition); it != schedules.on_transition.end()) {
            _run_transition_schedule(it->second);
        }

        _apply_commands();

        /* 3. Update the state */
        state_res->_previous = current;
        state_res->_current = next;

        /* 4. Execute OnEnter for the `next` state */
        if (auto it = schedules.on_enter.find(static_cast<usize>(next)); it != schedules.on_enter.end()) {
            _run_transition_schedule(it->second);
        }

        _apply_commands();

        next_state_res->next.reset();
    };

    return *this;
}

template<auto... SystemFuncs, typename ScheduleLabel>
r::Application::SystemConfigurator r::Application::add_systems(ScheduleLabel label)
{
    ScheduleGraph *target_graph = nullptr;

    if constexpr (std::is_same_v<ScheduleLabel, Schedule>) {
        target_graph = &_systems[label];
    } else if constexpr (detail::is_on_enter<ScheduleLabel>::value) {
        using StateEnum = typename ScheduleLabel::EnumType;
        auto &state_schedules = _states[typeid(StateEnum)];
        target_graph = &state_schedules.on_enter[static_cast<usize>(label.state)];
    } else if constexpr (detail::is_on_exit<ScheduleLabel>::value) {
        using StateEnum = typename ScheduleLabel::EnumType;
        auto &state_schedules = _states[typeid(StateEnum)];
        target_graph = &state_schedules.on_exit[static_cast<usize>(label.state)];
    } else if constexpr (detail::is_on_transition<ScheduleLabel>::value) {
        using StateEnum = typename ScheduleLabel::EnumType;
        auto &state_schedules = _states[typeid(StateEnum)];
        typename States::Transition transition{static_cast<usize>(label.from), static_cast<usize>(label.to)};
        target_graph = &state_schedules.on_transition[transition];
    } else {
        static_assert(always_false<ScheduleLabel>,
            "Unsupported type provided to add_systems. Must be a Schedule enum or a state event like OnEnter{...}.");
    }

    std::vector<SystemTypeId> ids;
    (ids.push_back(_add_one_system_to_graph<SystemFuncs>(*target_graph)), ...);

    return SystemConfigurator(this, target_graph, std::move(ids));
}
