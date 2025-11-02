---
sidebar_position: 9
---

# API Reference

Complete API reference for R-Engine ECS.

## Core Classes

### Scene

The main ECS world that manages entities, components, and resources.

[Scene API →](./scene.md)

### Commands

Deferred command buffer for modifying the ECS world.

[Commands API →](./commands.md)

### EntityCommands

Commands for a specific entity.

[EntityCommands API →](./entity-commands.md)

### Query with Wrappers

Type-safe entity iteration with component access.

[Query API →](./query.md)

## Event System

### EventWriter\<T\>

Send events to other systems.

### EventReader\<T\>

Receive events from other systems.

[Events API →](./events.md)

## Quick Reference

| Type | Purpose | Example |
|------|---------|---------|
| `Entity` | Entity identifier | `Entity player = ...;` |
| `Ref<T>` | Read-only component access | `Ref<Position>` |
| `Mut<T>` | Mutable component access | `Mut<Position>` |
| `With<T>` | Filter: has component | `With<Player>` |
| `Without<T>` | Filter: lacks component | `Without<Dead>` |
| `Res<T>` | Read-only resource | `Res<Config>` |
| `ResMut<T>` | Mutable resource | `ResMut<Score>` |
| `Commands` | Command buffer | `Commands& cmds` |
| `EventWriter<T>` | Send events | `EventWriter<E>` |
| `EventReader<T>` | Read events | `EventReader<E>` |

## Usage Patterns

### System Signature

```cpp
void system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<Time> time,
    Commands& commands,
    EventWriter<Event> events
) {
    // System logic
}
```

### Query Iteration

```cpp
for (auto [pos, vel] : query) {
    pos.ptr->x += vel.ptr->x;
}
```

### Command Usage

```cpp
commands.spawn(Position{}, Velocity{});
commands.entity(e).insert(Health{100});
commands.insert_resource(Config{});
```

## Next Steps

Browse detailed API documentation for each class.
