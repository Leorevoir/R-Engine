---
sidebar_position: 2
---

# Commands API

Commands provide deferred modifications to the ECS world.

## Methods

### spawn()

```cpp
EntityCommands spawn()
EntityCommands spawn(Components... components)
```

Queue entity creation.

**Returns**: EntityCommands for further configuration

### entity()

```cpp
EntityCommands entity(Entity e)
```

Get commands for a specific entity.

### insert_resource()

```cpp
void insert_resource(T resource)
```

Queue resource insertion.

### remove_resource()

```cpp
void remove_resource<T>()
```

Queue resource removal.

## Example Usage

```cpp
void system(Commands& commands) {
    // Spawn entities
    Entity e = commands.spawn(Position{}, Velocity{}).id();
    
    // Modify entities
    commands.entity(e).insert(Health{100});
    
    // Manage resources
    commands.insert_resource(GameState{});
}
```

## When Commands Execute

Commands are applied between system executions:

```
System A runs → commands queued
System B runs → commands queued
→ Commands applied here
System C runs → sees changes
```

## See Also

- [EntityCommands](./entity-commands.md) - Per-entity commands
- [Scene](./scene.md) - Direct world access
