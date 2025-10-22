---
sidebar_position: 6
---

# Commands

Commands provide deferred access to modify the ECS world. They prevent iterator invalidation and enable safe structural changes.

## What are Commands?

Commands queue modifications to be applied later, ensuring that queries don't break during iteration.

```cpp
void system(Query<Entity> query, Commands& commands) {
    for (auto [entity] : query) {
        // Safe: queued for later
        commands.entity(entity).despawn();
    }
    // Commands applied after iteration
}
```

## Command Types

### Spawn Entity

```cpp
void system(Commands& commands) {
    // Empty entity
    Entity e = commands.spawn().id();
    
    // With components
    commands.spawn(
        Position{0, 0},
        Velocity{1, 0}
    );
}
```

### Entity Commands

```cpp
void system(Entity entity, Commands& commands) {
    auto entity_cmds = commands.entity(entity);
    
    // Add component
    entity_cmds.insert(Health{100});
    
    // Remove component
    entity_cmds.remove<AI>();
    
    // Despawn
    entity_cmds.despawn();
}
```

### Resource Commands

```cpp
void system(Commands& commands) {
    // Insert resource
    commands.insert_resource(Config{});
    
    // Remove resource
    commands.remove_resource<OldResource>();
}
```

## EntityCommands

Chain operations on a single entity:

```cpp
commands.spawn()
    .insert(Position{0, 0})
    .insert(Velocity{1, 0})
    .insert(Player{});
```

## Command Execution

Commands are applied between system executions:

```
System A executes
  → Commands queued
System B executes
  → Commands queued
Commands applied ← All at once
System C executes
  → Can see changes
```

## Best Practices

### ✅ Do

- Use commands for all structural changes
- Chain entity commands for clarity
- Let commands apply automatically

### ❌ Don't

- Don't try to apply commands manually
- Don't expect immediate changes within the same system

## Example

```cpp
void spawn_enemies(Commands& commands) {
    for (int i = 0; i < 10; i++) {
        commands.spawn(
            Enemy{},
            Position{rand() % 800, rand() % 600},
            Health{50}
        );
    }
}

void cleanup(Query<Entity, With<Dead>> query, Commands& commands) {
    for (auto [entity, _] : query) {
        commands.entity(entity).despawn();
    }
}
```

## Next Steps

- See [Advanced Features](../advanced/index.md)
- Check [Examples](../examples/index.md)
