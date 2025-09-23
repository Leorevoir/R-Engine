#pragma once

#include <R-Engine/ECS/Resolver.hpp>
#include <R-Engine/Types.hpp>

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace r {

namespace ecs {

class CommandBuffer;

/**
 * @brief type trait to extract the argument list from any callable.
 *
 * supports:
 *   - lambdas / functors (operator())
 *   - function pointers
 *   - member function pointers
 *
 * result is available as `function_traits<F>::args` (a std::tuple of argument types).
 */
template<typename T>
struct function_traits : function_traits<decltype(&T::operator())> {
};

/**
* @brief const operator ()
* @info (for lambdas and functors)
*
* void operator() (Args...) const;
*/
template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const> {
        using args = std::tuple<std::remove_cvref_t<Args>...>;
};

/**
* @brief non-const operator ()
* @info (for lambdas and functors)
*
* void operator() (Args...);
*/
template<typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...)> {
        using args = std::tuple<std::remove_cvref_t<Args>...>;
};

/**
* @brief function pointer
* @info (for free functions)
*
* void (*)(int, float);
*/
template<typename R, typename... Args>
struct function_traits<R (*)(Args...)> {
        using args = std::tuple<std::remove_cvref_t<Args>...>;
};

/**
 * @brief invoke a system function with arguments resolved from the ECS Scene.
 *
 * for each argument type in the function signature, uses Resolver to construct
 * the correct wrapper (Res<T>, ResMut<T>, Query<...>, etc).
 *
 * @param func system function type
 * @param args argument types (deduced from function_traits)
 */

template<typename Func, typename... Args, size_t... I>
static inline auto call_with_resolved(Func &&f, Scene &scene, CommandBuffer &cmd, std::tuple<Args...>, std::index_sequence<I...>)
{
    Resolver resolver(&scene, &cmd);
    auto resolved_args = std::make_tuple(resolver.resolve(std::type_identity<Args>{})...);
    return std::apply(std::forward<Func>(f), resolved_args);
}

/**
 * @brief entry point to execute a system.
 *
 * deduces the system function's argument list via function_traits,
 * and calls it with arguments automatically provided by Resolver.
 *
 * this allows writing systems like:
 *   void my_system(Res<Time>, Commands cmds, Query<Mut<Position>, Ref<Velocity>>);
 *
 * without manually wiring dependencies.
 */

template<typename Func>
static inline void run_system(Func &&f, Scene &scene, CommandBuffer &cmd)
{
    using traits = function_traits<std::remove_cvref_t<Func>>;
    using args = typename traits::args;

    call_with_resolved(std::forward<Func>(f), scene, cmd, args{}, std::make_index_sequence<std::tuple_size_v<args>>{});
}

}// namespace ecs

}// namespace r
