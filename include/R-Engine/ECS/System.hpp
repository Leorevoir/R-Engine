#pragma once

#include <R-Engine/ECS/Resolver.hpp>
#include <R-Engine/Systems/ScheduleGraph.hpp>
#include <R-Engine/Types.hpp>

#include <tuple>
#include <type_traits>
#include <utility>

#ifndef R_UNUSED
    #define R_UNUSED __attribute__((unused))
#endif /* R_UNUSED */

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
 * @brief noexcept function pointer
 * @info (for free functions with noexcept)
 *
 * void (*)(int, float) noexcept;
 */
template<typename R, typename... Args>
struct function_traits<R (*)(Args...) noexcept> {
        using args = std::tuple<std::remove_cvref_t<Args>...>;
};

template<auto Func>
void get_system_access(sys::Access &comp_access, sys::Access &res_access);

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
static inline auto call_with_resolved(Func &&f, Scene &scene, CommandBuffer &cmd, std::tuple<Args...>, std::index_sequence<I...>);

/**
 * @brief invoke a predicate function with arguments resolved from the ECS Scene.
 *
 * similar to `call_with_resolved`, but specifically for predicates that return a boolean.
 *
 * @param p predicate function type
 * @param args argument types (deduced from function_traits)
 * @return the boolean result of the predicate.
 */
template<typename Predicate, typename... Args, size_t... I>
static inline bool call_predicate_with_resolved(Predicate &&p, Scene &scene, CommandBuffer &cmd, std::tuple<Args...>,
    std::index_sequence<I...>);

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
static inline void run_system(Func &&f, Scene &scene, CommandBuffer &cmd);

}// namespace ecs

}// namespace r

#include "Inline/System.inl"

#ifdef R_UNUSED
    #undef R_UNUSED
#endif /* R_UNUSED */
