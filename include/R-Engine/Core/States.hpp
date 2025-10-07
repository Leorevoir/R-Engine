#pragma once

#include <optional>
#include <type_traits>

namespace r {

/**
 * @brief Enum to define when the system will be enabled
 */
enum class StateTrigger {
    OnEnter,
    OnExit,
    OnTransition,
};

/**
 * @brief State struct
 */
template<typename T>
struct State {
    static_assert(std::is_enum_v<T>, "State type must be an enum.");
    State(T initial) : _current(initial) {}

    T current() const { return _current; }
    std::optional<T> previous() const { return _previous; }

private:
    friend class Application;
    T _current;
    std::optional<T> _previous = std::nullopt;
};

/**
 * @brief Next state to add
 */
template<typename T>
struct NextState {
    static_assert(std::is_enum_v<T>, "State type must be an enum.");
    std::optional<T> next = std::nullopt;

    void set(T next_state) {
        next = next_state;
    }
};

template<typename T>
struct StateCondition {
    static_assert(std::is_enum_v<T>, "State type must be an enum.");
    StateTrigger trigger;
    T state_to;
    std::optional<T> state_from = std::nullopt;
};

template<typename T>
StateCondition<T> OnEnter(T state) {
    return {StateTrigger::OnEnter, state};
}

template<typename T>
StateCondition<T> OnExit(T state) {
    return {StateTrigger::OnExit, state};
}

template<typename T>
StateCondition<T> OnTransition(T from, T to) {
    return {StateTrigger::OnTransition, to, from};
}

} // namespace r