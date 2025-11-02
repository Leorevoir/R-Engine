---
sidebar_position: 2
---

# Commands API

`ecs::Commands` provide deferred modifications to the ECS world.

## Methods

### spawn()

```cpp
EntityCommands spawn() noexcept;
EntityCommands spawn(Components&&... components) noexcept;
```

Schedules an entity to be created. You can optionally provide initial components.

**Returns**: `EntityCommands` for further configuration.

### entity()

```cpp
EntityCommands entity(Entity e) noexcept;
```

Get an `EntityCommands` handle for an existing entity to modify it.

### insert_resource()

```cpp
void insert_resource(T res) noexcept;
```

Schedules a resource to be inserted into the scene. If a resource of this type already exists, it will be overwritten.

### remove_resource()

```cpp
void remove_resource<T>() noexcept;
```

Schedules a resource to be removed from the scene.

### despawn()

```cpp
void despawn(Entity e) noexcept;
```

Schedules an entity and all of its descendants to be despawned.

## Example Usage

```cpp
void system(ecs::Commands& commands) {
    // Spawn entities
    ecs::Entity e = commands.spawn(Position{}, Velocity{}).id();

    // Modify an existing entity
    commands.entity(e).insert(Health{100});

    // Manage resources
    commands.insert_resource(GameState{});

    // Despawn an entity
    commands.despawn(e);
}
```

## When Commands Execute

Commands are queued during system execution and are all applied at specific synchronization points, typically after a schedule completes. This prevents iterator invalidation and ensures a consistent world state for all systems running within the same stage.

```
System A runs → commands queued
System B runs → commands queued
→ Sync point: All queued commands are applied here
System C runs → sees the changes from A and B
```

## See Also

- [EntityCommands](./entity-commands.md) - For chaining modifications to a single entity.
- [Scene](./scene.md) - The underlying container for all ECS data.
