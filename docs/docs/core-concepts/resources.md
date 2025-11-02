---
sidebar_position: 4
---

# Resources

Resources are unique, global data accessible by all systems. They are perfect for storing configuration, global state, or services.

## What is a Resource?

A resource is a singleton piece of data that exists outside of entities. Unlike components which are attached to entities, resources are global and unique.

```cpp
struct GameConfig {
    float gravity = 9.81f;
    int max_enemies = 100;
    bool debug_mode = false;
};
```

## Adding Resources

### At Application Startup

```cpp
Application{}
    .insert_resource(GameConfig{})
    .insert_resource(DeltaTime{0.016f})
    .run();
```

### From Systems

```cpp
void system(Commands& commands) {
    commands.insert_resource(NewResource{42});
}
```

## Accessing Resources

### Read-Only Access

Use `Res<T>` for immutable access:

```cpp
void physics_system(
    Query<Mut<Velocity>> query,
    Res<GameConfig> config
) {
    for (auto [vel] : query) {
        vel->y += config->gravity;
    }
}
```

### Mutable Access

Use `ResMut<T>` for mutable access:

```cpp
void score_system(
    ResMut<Score> score,
    EventReader<PointEvent> events
) {
    for (const auto& event : events) {
        score->value += event.points;
    }
}
```

## Removing Resources

```cpp
void cleanup_system(Commands& commands) {
    commands.remove_resource<TemporaryData>();
}
```

## Common Resource Patterns

### Configuration

```cpp
struct GameSettings {
    float volume = 1.0f;
    bool fullscreen = false;
    int difficulty = 1;
};

// Use in systems
void apply_settings(Res<GameSettings> settings) {
    set_volume(settings->volume);
    set_fullscreen(settings->fullscreen);
}
```

### Delta Time

```cpp
struct DeltaTime {
    float dt;
    float elapsed;
};

void time_system(ResMut<DeltaTime> time) {
    // Update each frame
    time->dt = calculate_delta();
    time->elapsed += time->dt;
}
```

### Input State

```cpp
struct Input {
    bool keys;
    int mouse_x, mouse_y;
    bool mouse_buttons;
};

void input_system(ResMut<Input> input) {
    // Update input state
    poll_events(input.ptr);
}
```

## Best Practices

### ✅ Do

- Use resources for global, unique data
- Prefer `Res<T>` over `ResMut<T>` when possible
- Use resources for services and configuration

### ❌ Don't

- Don't use resources for entity-specific data (use components)
- Don't abuse resources for everything (use components when appropriate)

## Next Steps

- Learn about [Queries](./queries.md)
- Explore [Commands](./commands.md)
- See [Examples](../examples/index.md)
