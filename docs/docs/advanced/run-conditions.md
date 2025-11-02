---
sidebar_position: 3
---

# Run Conditions

Run conditions control when systems execute based on runtime state.

## What are Run Conditions?

Run conditions are predicate functions that return `bool`, determining if a system (or group of systems) should run in a given frame.

```cpp
// Define a state enum
enum class GameStateEnum { Paused, Running };

// Define a condition that checks the state
bool is_running(ecs::Res<State<GameStateEnum>> state) {
    return state.ptr && state.ptr->current() == GameStateEnum::Running;
}

// Apply the condition to systems
app.add_systems<player_movement, enemy_ai>(Schedule::UPDATE)
   .run_if<is_running>();
```

## Chaining Conditions

You can chain conditions using logical operators.

- `.run_if<P>()`: Replaces any existing condition.
- `.run_unless<P>()`: A negated `run_if`. Runs if the predicate is false.
- `.run_and<P>()`: Adds a condition that must also be true (AND).
- `.run_or<P>()`: Adds a condition that can also be true (OR).

```cpp
.add_systems<special_power_system>(Schedule::UPDATE)
    .run_if<is_running>()
    .run_and<player_has_mana>()
    .run_unless<player_is_stunned>();
```

## Built-in Conditions

The engine provides several common run conditions in the `r::run_conditions` namespace.

### `in_state<StateValue>`

Checks if a state machine is in a specific state.

````cpp
// Runs only when the game is in the 'Playing' state.
.run_if<run_conditions::in_state<AppState::Playing>>()```

### `state_changed<T>`

Runs for one update cycle when the state of type `T` changes.

```cpp
// Runs only on the frame the AppState changes.
.run_if<run_conditions::state_changed<AppState>>()
````

### `on_event<T>`

Runs if any events of type `T` were sent this frame.

```cpp
// Runs only if a PlayerDeathEvent was fired.
.run_if<run_conditions::on_event<PlayerDeathEvent>>()
```

### `resource_exists<T>`

Runs if a resource of type `T` exists.

```cpp
.run_if<run_conditions::resource_exists<SpecialPowerup>>()
```

## Custom Conditions

Any function that can be a system can also be a run condition, as long as it returns `bool`.

```cpp
bool has_enemies(ecs::Query<ecs::With<Enemy>> query) {
    return query.size() > 0;
}

.add_systems<spawn_wave>(Schedule::UPDATE)
   .run_unless<has_enemies>(); // run_if<...>() works too!
```

## Performance

Run conditions are evaluated every frame before their systems run. Keep them lightweight.

```cpp
// ✓ Good: Simple check, very fast.
bool is_ready(ecs::Res<State> state) {
    return state.ptr->ready;
}

// ✗ Bad: Iterates over many components, potentially slow.
bool expensive_check(ecs::Query<ecs::Ref<LargeData>> query) {
    for (auto [data] : query) {
        // Complex computation...
    }
    return true;
}
```

## Next Steps

- Learn about [Schedules](./schedules.md)
- Check [Examples](../examples/index.md)
