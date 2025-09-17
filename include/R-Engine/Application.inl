#pragma once

#include <R-Engine/Plugins/Plugin.hpp>

#include <type_traits>

// clang-format off

template<typename Func>
r::Application &r::Application::add_system(Schedule when, Func &&func) noexcept
{
    using FuncDecay = std::decay_t<Func>;
    FuncDecay fn = std::forward<Func>(func);

    _systems[when].emplace_back(
    [fn](ecs::Scene &scene) mutable
    {
        ecs::run_system(fn, scene);
    });

    return *this;
}

// clang-format on

template<typename ResT>
r::Application &r::Application::insert_resource(ResT res) noexcept
{
    _scene.insert_resource<ResT>(std::move(res));
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
