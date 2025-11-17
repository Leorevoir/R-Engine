#pragma once

/**
 * System details traits
 */

namespace r {

namespace ecs {

namespace detail {

template<typename T, typename = void>
struct system_param_access {
        static void get(sys::Access R_UNUSED &comp_access, sys::Access R_UNUSED &res_access)
        {
            (void) comp_access;
            (void) res_access;
        }
};

template<typename T>
struct system_param_access<T, std::enable_if_t<is_res<T>::value>> {
        static void get(sys::Access R_UNUSED &comp_access, sys::Access &res_access)
        {
            (void) comp_access;
            using ResType = typename T::ResourceType;
            res_access.reads.insert(typeid(ResType));
        }
};

template<typename T>
struct system_param_access<T, std::enable_if_t<is_resmut<T>::value>> {
        static void get(sys::Access R_UNUSED &comp_access, sys::Access &res_access)
        {
            (void) comp_access;
            using ResType = typename T::ResourceType;
            res_access.writes.insert(typeid(ResType));
        }
};

template<typename W>
void get_query_wrapper_access(sys::Access &comp_access)
{
    if constexpr (is_mut<W>::value) {
        comp_access.writes.insert(typeid(typename component_of<W>::type));
    } else if constexpr (is_ref<W>::value || is_optional<W>::value) {
        comp_access.reads.insert(typeid(typename component_of<W>::type));
    }
}

template<typename... T>
struct system_param_access<Query<T...>> {
        static void get(sys::Access &comp_access, sys::Access R_UNUSED &res_access)
        {
            (void) res_access;
            (get_query_wrapper_access<T>(comp_access), ...);
        }
};
}// namespace detail

/**
* System implementation
*/

template<auto Func>
void get_system_access(sys::Access &comp_access, sys::Access &res_access)
{
    using traits = function_traits<std::remove_cvref_t<decltype(Func)>>;
    using args_tuple = typename traits::args;
    std::apply([&](auto... args) { (detail::system_param_access<decltype(args)>::get(comp_access, res_access), ...); }, args_tuple{});
}

template<typename Func, typename... Args, size_t... I>
static inline auto call_with_resolved(Func &&f, Scene &scene, CommandBuffer &cmd, std::tuple<Args...>, std::index_sequence<I...>)
{
    Resolver resolver(&scene, &cmd);
    auto resolved_args = std::make_tuple(resolver.resolve(std::type_identity<Args>{})...);
    return std::apply(std::forward<Func>(f), resolved_args);
}

template<typename Predicate, typename... Args, size_t... I>
static inline bool call_predicate_with_resolved(Predicate &&p, Scene &scene, CommandBuffer &cmd, std::tuple<Args...>,
    std::index_sequence<I...>)
{
    Resolver resolver(&scene, &cmd);
    auto resolved_args = std::make_tuple(resolver.resolve(std::type_identity<Args>{})...);
    return static_cast<bool>(std::apply(std::forward<Predicate>(p), resolved_args));
}

template<typename Func>
static inline void run_system(Func &&f, Scene &scene, CommandBuffer &cmd)
{
    using traits = function_traits<std::remove_cvref_t<Func>>;
    using args = typename traits::args;

    call_with_resolved(std::forward<Func>(f), scene, cmd, args{}, std::make_index_sequence<std::tuple_size_v<args>>{});
}

}// namespace ecs

}// namespace r
