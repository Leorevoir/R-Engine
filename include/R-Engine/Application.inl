#pragma once

#include <R-Engine/Application.hpp>
#include <R-Engine/ECS/System.hpp>
#include <R-Engine/Plugins/Plugin.hpp>

#include <type_traits>
#include <utility>

namespace r::detail {
template<typename>
struct is_on_enter : std::false_type {
};
template<typename T>
struct is_on_enter<OnEnter<T>> : std::true_type {
};

template<typename>
struct is_on_exit : std::false_type {
};
template<typename T>
struct is_on_exit<OnExit<T>> : std::true_type {
};

template<typename>
struct is_on_transition : std::false_type {
};
template<typename T>
struct is_on_transition<OnTransition<T>> : std::true_type {
};
}// namespace r::detail

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
r::sys::SystemSetId r::Application::_ensure_set_exists(sys::ScheduleGraph &graph) noexcept
{
    const sys::SystemSetId id = typeid(sys::SystemSetTag<SetType>);

    if (graph.sets.find(id) == graph.sets.end()) {
        graph.sets.emplace(id, sys::SystemSet(typeid(SetType).name(), id));
    }
    return id;
}

template<typename... SetTypes>
auto r::Application::configure_sets(Schedule when) noexcept -> sys::SetConfigurator<SetTypes...>
{
    auto &graph = _systems[when];
    std::vector<sys::SystemSetId> ids = {(_ensure_set_exists<SetTypes>(graph))...};

    return sys::SetConfigurator<SetTypes...>(this, when, std::move(ids));
}

template<typename ResT>
r::Application &r::Application::insert_resource(ResT &&res) noexcept
{
    _scene.insert_resource(std::forward<ResT>(res));
    return *this;
}

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
r::sys::SystemTypeId r::Application::_add_one_system_to_graph(sys::ScheduleGraph &graph) noexcept
{
    sys::SystemTypeId id(typeid(sys::SystemTag<SystemFunc>));

    sys::SystemNode node(id.name(), id, &system_invoker_template<SystemFunc>, {});

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
    insert_resource(NextState<T>{.next = initial_state});

    _state_transition_runner = [this]() {
        auto *state_res = _scene.get_resource_ptr<State<T>>();
        auto *next_state_res = _scene.get_resource_ptr<NextState<T>>();

        if (!next_state_res || !next_state_res->next.has_value()) {
            if (state_res) {
                state_res->_previous.reset();
            }
            return;
        }

        T next = next_state_res->next.value();
        T current = state_res->current();

        const bool is_initial_transition = !state_res->previous().has_value();

        if (current == next && !is_initial_transition) {
            next_state_res->next.reset();
            return;
        }

        auto &schedules = _states[typeid(T)];

        if (current != next) {
            /* 1. Execute OnExit for the `current` state */
            if (auto it = schedules.on_exit.find(static_cast<usize>(current)); it != schedules.on_exit.end()) {
                _run_transition_schedule(it->second);
            }

            /* 2. Execute OnTransition */
            typename sys::States::Transition transition{static_cast<usize>(current), static_cast<usize>(next)};
            if (auto it = schedules.on_transition.find(transition); it != schedules.on_transition.end()) {
                _run_transition_schedule(it->second);
            }

            _apply_commands();
        }

        /* 3. Update the state */
        if (current != next) {
            state_res->_previous = current;
        }
        state_res->_current = next;

        /* 4. Execute OnEnter for the `next` state */
        if (auto it = schedules.on_enter.find(static_cast<usize>(next)); it != schedules.on_enter.end()) {
            _run_transition_schedule(it->second);
        }

        _apply_commands();
        _run_schedule(Schedule::EVENT_CLEANUP);

        next_state_res->next.reset();
    };

    return *this;
}

template<auto... SystemFuncs, typename ScheduleLabel>
r::sys::SystemConfigurator r::Application::add_systems(ScheduleLabel label)
{
    sys::ScheduleGraph *target_graph = nullptr;

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
        typename sys::States::Transition transition{static_cast<usize>(label.from), static_cast<usize>(label.to)};
        target_graph = &state_schedules.on_transition[transition];
    } else {
        static_assert(always_false<ScheduleLabel>,
            "Unsupported type provided to add_systems. Must be a Schedule enum or a state event like OnEnter{...}.");
    }

    std::vector<sys::SystemTypeId> ids;
    (ids.push_back(_add_one_system_to_graph<SystemFuncs>(*target_graph)), ...);

    return sys::SystemConfigurator(this, target_graph, std::move(ids));
}
