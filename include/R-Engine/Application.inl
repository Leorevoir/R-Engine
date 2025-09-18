#pragma once

#include "R-Engine/Application.hpp"
#include <R-Engine/Plugins/Plugin.hpp>

#include <type_traits>

// clang-format off

template<typename Func>
void r::Application::_add_one_system(r::Schedule when, Func &&func) noexcept
{
    using FuncDecay = std::decay_t<Func>;
    FuncDecay fn = std::forward<Func>(func);

    _systems[when].emplace_back(
    [fn](ecs::Scene &scene, ecs::CommandBuffer &cmd) mutable
    {
        ecs::run_system(fn, scene, cmd);
    });
}

template<typename... Funcs>
r::Application &r::Application::add_systems(Schedule when, Funcs &&...funcs) noexcept
{
    (_add_one_system(when, std::forward<Funcs>(funcs)), ...);
    return *this;
}

// clang-format on

template<typename ResT>
r::Application &r::Application::insert_resource(ResT res) noexcept
{
    _scene.insert_resource<ResT>(std::move(res));
    return *this;
}

template<typename PluginT>
r::Application &r::Application::add_plugins(PluginT &&plugin) noexcept
{
    using DecayedPluginT = std::decay_t<PluginT>;
    if constexpr (std::is_base_of_v<Plugin, DecayedPluginT> || std::is_base_of_v<PluginGroup, DecayedPluginT>) {
        plugin.build(*this);
    } else {
        static_assert(always_false<PluginT>, "Argument must be a Plugin or PluginGroup instance.");
    }
    return *this;
}

template<typename PluginT, typename... Args>
r::Application &r::Application::add_plugins(Args &&...args) noexcept
{
    if constexpr (std::is_base_of_v<Plugin, PluginT>) {
        PluginT plugin{std::forward<Args>(args)...};
        plugin.build(*this);
    } else if constexpr (std::is_base_of_v<PluginGroup, PluginT>) {
        PluginT group{std::forward<Args>(args)...};
        group.build(*this);
    } else {
        static_assert(always_false<PluginT>, "PluginT must be a Plugin or a PluginGroup");
    }
    return *this;
}

template<typename PluginT1, typename PluginT2, typename... Rest>
r::Application &r::Application::add_plugins() noexcept
{
    add_plugins<PluginT1>();
    add_plugins<PluginT2>();
    if constexpr (sizeof...(Rest) > 0) {
        add_plugins<Rest...>();
    }
    return *this;
}
