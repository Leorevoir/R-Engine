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
        State(T initial) : _current(initial)
        {
        }

        T current() const
        {
            return _current;
        }
        std::optional<T> previous() const
        {
            return _previous;
        }

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

        void set(T next_state)
        {
            next = next_state;
        }
};

template<typename T>
struct OnEnter {
        static_assert(std::is_enum_v<T>, "State type must be an enum.");
        using EnumType = T;
        T state;
};

template<typename T>
struct OnExit {
        static_assert(std::is_enum_v<T>, "State type must be an enum.");
        using EnumType = T;
        T state;
};

template<typename T>
struct OnTransition {
        static_assert(std::is_enum_v<T>, "State type must be an enum.");
        using EnumType = T;
        T from;
        T to;
};

}// namespace r
