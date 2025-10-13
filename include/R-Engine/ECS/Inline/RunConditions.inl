#pragma once

namespace r {
namespace run_conditions {

template<auto StateValue>
inline bool in_state(ecs::Res<State<typename std::remove_cvref_t<decltype(StateValue)>>> state)
{
    using StateEnum = typename std::remove_cvref_t<decltype(StateValue)>;
    static_assert(std::is_enum_v<StateEnum>, "in_state must be used with an enum value.");
    return state.ptr && state.ptr->current() == StateValue;
}

template<typename T>
inline bool state_changed(ecs::Res<State<T>> state)
{
    static_assert(std::is_enum_v<T>, "State type must be an enum.");
    return state.ptr && state.ptr->previous().has_value();
}

template<typename T>
inline bool on_event(ecs::EventReader<T> reader)
{
    return reader.has_events();
}

template<typename T>
inline bool resource_exists(ecs::Res<T> res)
{
    return res.ptr != nullptr;
}

}// namespace run_conditions
}// namespace r
