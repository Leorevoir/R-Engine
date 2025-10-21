#pragma once

template<typename... SetTypes>
r::sys::SetConfigurator<SetTypes...>::SetConfigurator(Application *app, Schedule schedule, std::vector<SystemSetId> setids) noexcept
    : ConfiguratorBase<SetConfigurator<SetTypes...>>(app), _schedule(schedule), _set_ids(std::move(setids))
{
    /* __ctor__ */
}

template<typename... SetTypes>
template<typename OtherSet>
auto r::sys::SetConfigurator<SetTypes...>::before() noexcept -> SetConfigurator &
{
    auto &graph = this->_app->_systems[_schedule];
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
auto r::sys::SetConfigurator<SetTypes...>::after() noexcept -> SetConfigurator &
{
    auto &graph = this->_app->_systems[_schedule];
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
