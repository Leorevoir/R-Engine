---
sidebar_position: 3
---

# Systems

Systems are functions that contain your application's logic. They operate on entities via queries and can access global resources.

## What is a System?

A system is a regular C++ function that takes ECS parameters and implements game logic:

```cpp
void movement_system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<DeltaTime> time
) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x * time->dt;
        pos->y += vel->y * time->dt;
    }
}
```

## System Parameters

Systems can accept various parameter types:

| Type | Description | Use Case |
|------|-------------|----------|
| `Query<...>` | Iterate over entities | Process multiple entities |
| `Res<T>` | Read-only resource access | Read configuration, time |
| `ResMut<T>` | Mutable resource access | Modify global state |
| `Commands` | Deferred command buffer | Spawn/despawn entities |
| `EventWriter<T>` | Send events | Asynchronous communication |
| `EventReader<T>` | Read events | React to events |

### Query Parameter

Access entity components:

```cpp
void system(Query<Mut<Position>, Ref<Velocity>, With<Player>> query) {
    for (auto [pos, vel, _] : query) {
        // Process player entities
    }
}
```

[Learn more about Queries →](./queries.md)

### Resource Parameters

Access global data:

```cpp
void system(
    Res<Config> config,      // Read-only
    ResMut<Score> score      // Mutable
) {
    if (score->value > config->high_score) {
        // New high score!
    }
}
```

[Learn more about Resources →](./resources.md)

### Commands Parameter

Modify the ECS world:

```cpp
void system(Commands& commands) {
    // Spawn entity
    commands.spawn(Position{}, Velocity{});
    
    // Insert resource
    commands.insert_resource(NewResource{});
}
```

[Learn more about Commands →](./commands.md)

### Event Parameters

Send and receive events:

```cpp
void system(
    EventWriter<CollisionEvent> writer,
    EventReader<InputEvent> reader
) {
    for (const auto& input : reader.iter()) {
        // Process input
    }
    
    writer.send(CollisionEvent{});
}
```

[Learn more about Events →](../advanced/events.md)

## Registering Systems

Add systems to schedules in your application:

```cpp
Application{}
    // STARTUP systems (run once)
    .add_systems<setup, init>(Schedule::STARTUP)
    
    // UPDATE systems (run each frame)
    .add_systems<input, movement, collision>(Schedule::UPDATE)
    
    // RENDER systems
    .add_systems<render_sprites>(Schedule::RENDER_2D)
    .run();
```

## System Ordering

### Automatic Ordering

By default, systems run in the order they're added:

```cpp
.add_systems<system_a, system_b, system_c>(Schedule::UPDATE)
// Runs: a → b → c
```

### Explicit Dependencies

Use `.after<>()` and `.before<>()` to specify order:

```cpp
.add_systems<input_system, movement_system>(Schedule::UPDATE)
.after<input_system>()  // movement_system runs after input_system

.add_systems<cleanup_system>(Schedule::UPDATE)
.before<render_system>()  // cleanup runs before render
```

### System Sets

Group related systems:

```cpp
.add_systems<physics_a, physics_b>(Schedule::UPDATE)
.in_set(SystemSet::Physics)

.add_systems<render_a, render_b>(Schedule::UPDATE)
.in_set(SystemSet::Rendering)
.after_set(SystemSet::Physics)  // All rendering after physics
```

[Learn more about System Sets →](../advanced/schedules.md)

## System Execution

### Single Execution

Systems run once per schedule execution:

```cpp
void system() {
    std::cout << "Running once per frame\n";
}
```

### Conditional Execution

Use run conditions to control when systems run:

```cpp
bool is_paused() {
    // Check pause state
    return /* ... */;
}

.add_systems<game_logic>(Schedule::UPDATE)
.run_if(not_run_condition<is_paused>())
```

[Learn more about Run Conditions →](../advanced/run-conditions.md)

## System Patterns

### Initialization System

Run once at startup:

```cpp
void setup_system(Commands& commands) {
    // Initialize resources
    commands.insert_resource(GameConfig{});
    
    // Spawn initial entities
    commands.spawn(Camera{}, Transform{});
    commands.spawn(Player{}, Position{}, Health{100});
}

// Register in STARTUP schedule
.add_systems<setup_system>(Schedule::STARTUP)
```

### Update System

Run every frame:

```cpp
void update_system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<DeltaTime> time
) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x * time->dt;
        pos->y += vel->y * time->dt;
    }
}

.add_systems<update_system>(Schedule::UPDATE)
```

### Event Handler System

React to events:

```cpp
void collision_handler(
    EventReader<CollisionEvent> events,
    Commands& commands
) {
    for (const auto& event : events.iter()) {
        // Handle collision
        if (event.damage > 0) {
            commands.entity(event.target).despawn();
        }
    }
}
```

### Cleanup System

Remove dead entities:

```cpp
void cleanup_dead_entities(
    Query<Entity, With<Dead>> query,
    Commands& commands
) {
    for (auto [entity, _] : query) {
        commands.entity(entity).despawn();
    }
}
```

## Best Practices

### ✅ Do

- Keep systems focused on one task
- Use queries to iterate over entities
- Prefer read-only access (`Ref<T>`) when possible
- Use commands for structural changes

```cpp
// Good: Focused system
void apply_gravity(Query<Mut<Velocity>> query, Res<Gravity> gravity) {
    for (auto [vel] : query) {
        vel->y += gravity->value;
    }
}
```

### ❌ Don't

- Don't store state in systems (use resources or components)
- Don't modify entities directly during query iteration
- Don't create circular dependencies

```cpp
// Bad: Storing state in system
void bad_system() {
    static int counter = 0;  // ❌ Use a resource instead
    counter++;
}
```

## Performance Tips

### Minimize Mutable Access

Use `Ref<T>` instead of `Mut<T>` when you don't need to modify:

```cpp
// Allows parallel execution with other read-only systems
void read_only_system(Query<Ref<Position>> query) {
    for (auto [pos] : query) {
        // Just reading
    }
}
```

### Split Large Systems

Break complex systems into smaller, focused ones:

```cpp
// Instead of one large system:
void game_logic() { /* physics, AI, input... */ }

// Split into focused systems:
void physics_system() { /* ... */ }
void ai_system() { /* ... */ }
void input_system() { /* ... */ }
```

### Use Filters

Filter queries to reduce iterations:

```cpp
// Only process active enemies
void ai_system(Query<Ref<Position>, With<Enemy>, Without<Dead>> query) {
    for (auto [pos, _] : query) {
        // AI logic for living enemies only
    }
}
```

## Example: Complete System Setup

```cpp
// Components
struct Position { float x, y; };
struct Velocity { float x, y; };
struct Player {};

// Resources
struct DeltaTime { float dt; };

// Systems
void movement_system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<DeltaTime> time
) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x * time->dt;
        pos->y += vel->y * time->dt;
    }
}

void input_system(
    Query<Mut<Velocity>, With<Player>> query,
    Res<Input> input
) {
    for (auto [vel, _] : query) {
        vel->x = input->horizontal * 5.0f;
        vel->y = input->vertical * 5.0f;
    }
}

// Application
int main() {
    Application{}
        .insert_resource(DeltaTime{})
        .insert_resource(Input{})
        .add_systems<input_system, movement_system>(Schedule::UPDATE)
        .after<input_system>()  // movement after input
        .run();
}
```

## Next Steps

- Learn about [Resources](./resources.md) for global state
- Explore [Queries](./queries.md) for entity access
- See [Commands](./commands.md) for world modification
- Check [Advanced Features](../advanced/index.md) for more system capabilities
