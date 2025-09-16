#pragma once

#include <type_traits>

// clang-format off

template<typename Func>
r::Application &r::Application::add_system(Schedule when, Func &&func)
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
r::Application &r::Application::insert_resource(ResT res)
{
    _scene.insert_resource<ResT>(std::move(res));
    return *this;
}
