#pragma once

template<typename Derived>
template<auto... SystemFuncs, typename ScheduleLabel>
r::sys::SystemConfigurator r::sys::ConfiguratorBase<Derived>::add_systems(ScheduleLabel label) noexcept
{
    return _app->add_systems<SystemFuncs...>(label);
}

template<typename Derived>
template<typename... SetTypes>
auto r::sys::ConfiguratorBase<Derived>::configure_sets(Schedule when) noexcept -> r::sys::SetConfigurator<SetTypes...>
{
    return _app->configure_sets<SetTypes...>(when);
}

template<typename Derived>
template<typename ResT>
Derived &r::sys::ConfiguratorBase<Derived>::insert_resource(ResT &&res) noexcept
{
    _app->insert_resource(std::forward<ResT>(res));
    return static_cast<Derived &>(*this);
}

template<typename Derived>
template<typename... Plugins>
Derived &r::sys::ConfiguratorBase<Derived>::add_plugins(Plugins &&...plugins) noexcept
{
    _app->add_plugins(std::forward<Plugins>(plugins)...);
    return static_cast<Derived &>(*this);
}

template<typename Derived>
void r::sys::ConfiguratorBase<Derived>::run()
{
    _app->run();
}
