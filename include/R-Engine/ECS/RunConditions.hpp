#pragma once

#include <R-Engine/Core/States.hpp>
#include <R-Engine/ECS/Event.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <type_traits>

namespace r {

namespace run_conditions {

/**
 * @brief A run condition that is true if the state machine is in the given state.
 * @tparam StateValue The enum value of the state to check for.
 * @param state The state resource provided by the scheduler.
 * @return True if the current state matches StateValue, false otherwise.
 *
 * @example
 * .run_if<in_state<AppState::Playing>>()
 */
template<auto StateValue>
bool in_state(ecs::Res<State<typename std::remove_cvref_t<decltype(StateValue)>>> state);

/**
 * @brief A run condition that is true for one update cycle when the state of type T changes.
 * @tparam T The enum type of the state machine.
 * @param state The state resource provided by the scheduler.
 * @return True if the state changed in this cycle, false otherwise.
 *
 * @example
 * .run_if<state_changed<AppState>>()
 */
template<typename T>
bool state_changed(ecs::Res<State<T>> state);

/**
 * @brief A run condition that is true if any events of type T have been sent this frame.
 * @tparam T The event type to check for.
 * @param reader The event reader resource provided by the scheduler.
 * @return True if there are one or more events of type T, false otherwise.
 *
 * @example
 * .run_if<on_event<MyEvent>>()
 */
template<typename T>
bool on_event(ecs::EventReader<T> reader);

/**
 * @brief A run condition that is true if a resource of type T exists.
 * @tparam T The resource type to check for.
 * @param res The resource provided by the scheduler.
 * @return True if the resource exists, false otherwise.
 *
 * @example
 * .run_if<resource_exists<MyResource>>()
 */
template<typename T>
bool resource_exists(ecs::Res<T> res);

}// namespace run_conditions

}// namespace r

#include "Inline/RunConditions.inl"
