#pragma once

inline r::sys::SystemConfigurator::SystemConfigurator(Application *app, ScheduleGraph *graph, std::vector<SystemTypeId> system_ids) noexcept
    : ConfiguratorBase(app), _graph(graph), _system_ids(std::move(system_ids))
{
    /* __ctor__ */
}

template<auto SystemFunc>
r::sys::SystemConfigurator &r::sys::SystemConfigurator::after() noexcept
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
r::sys::SystemConfigurator &r::sys::SystemConfigurator::after() noexcept
{
    SystemSetId dependency_set_id = _app->_ensure_set_exists<SetType>(*_graph);

    for (const auto &system_id : _system_ids) {
        _graph->nodes.at(system_id).after_sets.push_back(dependency_set_id);
    }
    _graph->dirty = true;
    return *this;
}

template<auto SystemFunc>
r::sys::SystemConfigurator &r::sys::SystemConfigurator::before() noexcept
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
r::sys::SystemConfigurator &r::sys::SystemConfigurator::before() noexcept
{
    SystemSetId dependent_set_id = _app->_ensure_set_exists<SetType>(*_graph);
    for (const auto &system_id : _system_ids) {
        _graph->nodes.at(system_id).before_sets.push_back(dependent_set_id);
    }
    _graph->dirty = true;
    return *this;
}

/**
* SystemConfigurator Implementation
*/

template<auto PredicateFunc>
r::sys::SystemConfigurator &r::sys::SystemConfigurator::run_if() noexcept
{
    _current_condition = _create_condition_wrapper<PredicateFunc>();
    _apply_condition();
    return *this;
}

template<auto PredicateFunc>
r::sys::SystemConfigurator &r::sys::SystemConfigurator::run_and() noexcept
{
    const auto new_condition = _create_condition_wrapper<PredicateFunc>();

    if (_current_condition) {
        _current_condition = [old_cond = _current_condition, new_cond = new_condition](
                                 ecs::Scene &scene) { return old_cond(scene) && new_cond(scene); };
    } else {
        _current_condition = new_condition;
    }

    _apply_condition();
    return *this;
}

template<auto PredicateFunc>
r::sys::SystemConfigurator &r::sys::SystemConfigurator::run_or() noexcept
{
    const auto new_condition = _create_condition_wrapper<PredicateFunc>();

    if (_current_condition) {
        _current_condition = [old_cond = _current_condition, new_cond = new_condition](
                                 ecs::Scene &scene) { return old_cond(scene) || new_cond(scene); };
    } else {
        _current_condition = new_condition;
    }

    _apply_condition();
    return *this;
}

template<auto PredicateFunc>
r::sys::SystemConfigurator &r::sys::SystemConfigurator::run_unless() noexcept
{
    _current_condition = _create_condition_wrapper<PredicateFunc>(true);
    _apply_condition();
    return *this;
}

/**
 * private
 */

template<auto PredicateFunc>
auto r::sys::SystemConfigurator::_create_condition_wrapper(bool negated) -> std::function<bool(ecs::Scene &)>
{
    using traits = ecs::function_traits<std::remove_cvref_t<decltype(PredicateFunc)>>;
    using args = typename traits::args;

    return [app = this->_app, negated](ecs::Scene &scene) -> bool {
        bool result = ecs::call_predicate_with_resolved(PredicateFunc, scene, app->_command_buffer, args{},
            std::make_index_sequence<std::tuple_size_v<args>>{});

        return negated ? !result : result;
    };
}

inline void r::sys::SystemConfigurator::_apply_condition()
{
    for (const auto &system_id : _system_ids) {
        _graph->nodes.at(system_id).condition = _current_condition;
    }
}

template<typename SetType>
r::sys::SystemConfigurator &r::sys::SystemConfigurator::in_set() noexcept
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
