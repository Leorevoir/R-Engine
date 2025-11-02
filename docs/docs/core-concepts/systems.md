---
sidebar_position: 3
---

# Systems

Systems are functions that contain your application's logic. They operate on entities by querying for components and can access global resources.

## What is a System?

A system is a regular C++ function that takes special ECS types as parameters. The engine uses these parameters to provide the necessary data from the `Scene`.

```cpp
// A system that moves entities based on their velocity.
void movement_system(
    ecs::Query<ecs::Mut<Position>, ecs::Ref<Velocity>> query,
    ecs::Res<FrameTime> time
) {
    for (auto [pos, vel] : query) {
        pos.ptr->x += vel.ptr->x * time.ptr->delta_time;
        pos.ptr->y += vel.ptr->y * time.ptr->delta_time;
    }
}
```

## System Parameters

Systems can accept various parameter types which are automatically injected by the engine:

| Type                  | Description                                     | Use Case                                       |
| --------------------- | ----------------------------------------------- | ---------------------------------------------- |
| `ecs::Query<...>`     | Iterate over entities with specific components. | The primary way to process multiple entities.  |
| `ecs::Res<T>`         | Read-only access to a global resource.          | Read configuration, time, input state.         |
| `ecs::ResMut<T>`      | Mutable access to a global resource.            | Modify global state like score or settings.    |
| `ecs::Commands`       | A buffer for deferred world modifications.      | Spawn/despawn entities, add/remove components. |
| `ecs::EventWriter<T>` | A sender for a specific event type.             | Asynchronous communication between systems.    |
| `ecs::EventReader<T>` | A receiver for a specific event type.           | Reacting to events sent by other systems.      |

[Learn more about Queries →](./queries.md)
[Learn more about Resources →](./resources.md)
[Learn more about Commands →](./commands.md)
[Learn more about Events →](../advanced/events.md)

## Registering Systems

You add systems to your application and assign them to a `Schedule`.

```cpp
Application{}
    // STARTUP systems run once at the beginning.
    .add_systems<setup_game, load_assets>(Schedule::STARTUP)

    // UPDATE systems run each frame.
    .add_systems<player_input, movement, collision>(Schedule::UPDATE)

    // RENDER systems handle drawing.
    .add_systems<render_sprites>(Schedule::RENDER_2D)
    .run();
```

## System Ordering

### Automatic Ordering

By default, the scheduler attempts to run systems in parallel if their data access does not conflict.

### Explicit Dependencies

Use `.after<>()` and `.before<>()` to define a specific execution order when needed.

```cpp
app.add_systems<movement_system>(Schedule::UPDATE)
   .after<input_system>(); // Ensures movement_system runs after input_system

app.add_systems<cleanup_system>(Schedule::UPDATE)
   .before<render_system>(); // Ensures cleanup runs before render
```

### System Sets

Group related systems into sets to manage their dependencies as a whole. Sets are defined by empty structs.

```cpp
struct PhysicsSet {};
struct RenderingSet {};

app.add_systems<physics_a, physics_b>(Schedule::UPDATE)
   .in_set<PhysicsSet>();

app.add_systems<render_a, render_b>(Schedule::UPDATE)
   .in_set<RenderingSet>();

// Configure the entire RenderingSet to run after the PhysicsSet
app.configure_sets<RenderingSet>(Schedule::UPDATE)
   .after<PhysicsSet>();
```

[Learn more about Schedules and Sets →](../advanced/schedules.md)

## Conditional Execution

Use run conditions to control when systems execute.

```cpp
// A predicate function that checks a resource.
bool is_not_paused(ecs::Res<GameState> state) {
    return !state.ptr->paused;
}

// The system will only run if the predicate returns true.
app.add_systems<game_logic>(Schedule::UPDATE)
   .run_if<is_not_paused>();
```

[Learn more about Run Conditions →](../advanced/run-conditions.md)

## Best Practices

### ✅ Do

- Keep systems focused on a single responsibility (e.g., a movement system, a rendering system).
- Use queries to iterate over entities.
- Prefer read-only access (`Ref<T>`, `Res<T>`) whenever possible to allow for more parallelism.
- Use `Commands` for all structural changes to the world.

```cpp
// Good: A focused system with clear dependencies.
void apply_gravity(ecs::Query<ecs::Mut<Velocity>> query, ecs::Res<Gravity> gravity) {
    for (auto [vel] : query) {
        vel.ptr->y += gravity.ptr->value;
    }
}
```

### ❌ Don't

- Don't store state within the system function itself (e.g., using `static` variables). Use resources or components instead.
- Don't modify the world directly while iterating (e.g., calling `scene.add_component`). Use `Commands`.
- Don't create circular dependencies between systems.

```cpp
// Bad: Storing state in a system.
void bad_system() {
    static int counter = 0;  // ❌ This is not thread-safe and is bad practice.
    counter++;               // Use a resource instead: ResMut<MyCounter>.
}
```

## Next Steps

- Learn about [Resources](./resources.md) for global state.
- Explore [Queries](./queries.md) for entity access.
- See [Commands](./commands.md) for world modification.
- Check [Advanced Features](../advanced/index.md) for more system capabilities.
