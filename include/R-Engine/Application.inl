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
