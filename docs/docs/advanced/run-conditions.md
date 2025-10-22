---
sidebar_position: 3
---

# Run Conditions

Run conditions control when systems execute based on runtime state.

## What are Run Conditions?

Run conditions are functions that return `bool`, determining if a system should run.

```cpp
// Define condition
bool is_paused(Res<GameState> state) {
    return state->paused;
}

// Apply condition
.add_systems<game_logic>(Schedule::UPDATE)
.run_if(not_run_condition<is_paused>())
```

## Built-in Conditions

### Resource Exists

```cpp
.run_if(resource_exists<Player>())
```

### Resource Equals

```cpp
bool game_running(Res<GameState> state) {
    return state->mode == GameMode::Playing;
}

.run_if(run_condition<game_running>())
```

## Custom Conditions

```cpp
bool has_enemies(Query<With<Enemy>> query) {
    return !query.is_empty();
}

.add_systems<spawn_wave>(Schedule::UPDATE)
.run_if(not_run_condition<has_enemies>())
```

## Combining Conditions

```cpp
.run_if(run_condition<is_playing>())
.run_if(not_run_condition<is_paused>())
```

## Common Patterns

### Pause System

```cpp
struct GameState {
    bool paused = false;
};

bool not_paused(Res<GameState> state) {
    return !state->paused;
}

.add_systems<movement, ai, physics>(Schedule::UPDATE)
.run_if(run_condition<not_paused>())
```

### Debug Mode

```cpp
bool debug_enabled(Res<Config> config) {
    return config->debug;
}

.add_systems<debug_draw, debug_info>(Schedule::UPDATE)
.run_if(run_condition<debug_enabled>())
```

## Performance

Run conditions are evaluated every frame. Keep them lightweight:

```cpp
// ✓ Good: Simple check
bool is_ready(Res<State> state) {
    return state->ready;
}

// ✗ Bad: Expensive operation
bool has_data(Query<Ref<LargeData>> query) {
    for (auto [data] : query) {
        // Complex computation...
    }
    return true;
}
```

## Next Steps

- Learn about [Schedules](./schedules.md)
- Check [Examples](../examples/index.md)
