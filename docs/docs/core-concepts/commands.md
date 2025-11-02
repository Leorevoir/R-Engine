---
sidebar_position: 6
---

# Commands

Commands provide deferred access to modify the ECS world. They prevent iterator invalidation and enable safe structural changes during system execution.

## What are Commands?

`ecs::Commands` is a system parameter that queues modifications (like spawning/despawning entities or adding/removing components). These changes are applied all at once at a safe point between system executions, ensuring that queries don't break while you're iterating over them.

```cpp
void system(ecs::Query<ecs::Entity, ecs::With<Enemy>> query, ecs::Commands& commands) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        ecs::Entity entity = it.entity();
        // This is safe: the despawn is queued and won't affect the current query iteration.
        commands.despawn(entity);
    }
    // All queued commands are applied after this system finishes.
}
```

## Command Types

### Spawning Entities

```cpp
void system(ecs::Commands& commands) {
    // Spawn an entity with no components
    ecs::Entity e = commands.spawn().id();

    // Spawn an entity with a set of components
    commands.spawn(
        Position{0, 0},
        Velocity{1, 0}
    );
}
```

### Modifying Entities

Use `commands.entity(entity_id)` to get an `EntityCommands` builder.

```cpp
void system(ecs::Entity entity, ecs::Commands& commands) {
    auto entity_cmds = commands.entity(entity);

    // Add a component
    entity_cmds.insert(Health{100});

    // Remove a component
    entity_cmds.remove<AI>();

    // Despawn the entity
    commands.despawn(entity);
}
```

### Managing Resources

```cpp
void system(ecs::Commands& commands) {
    // Insert or update a resource
    commands.insert_resource(Config{});

    // Remove a resource
    commands.remove_resource<OldResource>();
}
```

## EntityCommands

The `EntityCommands` struct provides a convenient builder pattern for chaining operations on a single entity:

```cpp
commands.spawn()
    .insert(Position{0, 0})
    .insert(Velocity{1, 0})
    .insert(Player{});
```

## Command Execution

Commands are applied at specific synchronization points, usually after all systems in a given schedule have run.

```
System A executes
  → Commands for spawning and component changes are queued
System B executes
  → More commands are queued
Sync Point (e.g., end of UPDATE schedule)
  → All queued commands are applied at once
System C executes
  → Can see the changes made by systems A and B
```

## Best Practices

### ✅ Do

- Use `Commands` for **all** structural changes (spawning, despawning, adding/removing components).
- Chain `EntityCommands` calls for better readability.
- Rely on the engine to apply commands automatically at the correct time.

### ❌ Don't

- Don't try to apply commands manually.
- Don't expect changes made via `Commands` to be visible within the same system that issued them.

## Example

```cpp
void spawn_enemies(ecs::Commands& commands) {
    for (int i = 0; i < 10; i++) {
        commands.spawn(
            Enemy{},
            Position{rand() % 800, rand() % 600},
            Health{50}
        );
    }
}

void cleanup_system(ecs::Query<ecs::Entity, ecs::With<Dead>> query, ecs::Commands& commands) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        commands.despawn(it.entity());
    }
}
```

## Next Steps

- See [Advanced Features](../advanced/index.md)
- Check [Examples](../examples/index.md)
