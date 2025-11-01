---
sidebar_position: 3
---

# Core Concepts

This section covers the fundamental building blocks of R-Engine ECS.

## Overview

R-Engine ECS is built on three main concepts:

### Entities

Entities are unique identifiers that represent "things" in your game or application. They are lightweight IDs that tie components together.

[Learn more about Entities →](./entities.md)

### Components

Components are pure data structures attached to entities. They define what an entity **is** or **has**.

[Learn more about Components →](./components.md)

### Systems

Systems are functions that contain logic. They define what your application **does** by operating on entities with specific components.

[Learn more about Systems →](./systems.md)

## Additional Concepts

### Resources

Resources are global, unique data accessible by all systems. Perfect for configuration, services, or shared state.

[Learn more about Resources →](./resources.md)

### Queries

Queries allow systems to iterate over entities that match specific criteria. They provide type-safe access to components.

[Learn more about Queries →](./queries.md)

### Commands

Commands provide deferred access to modify the ECS world. They prevent iterator invalidation and enable safe structural changes.

[Learn more about Commands →](./commands.md)

## The ECS Pattern

```cpp
// Components: Pure data
struct Position { float x, y; };
struct Velocity { float x, y; };

// Systems: Pure logic
void movement_system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<DeltaTime> time
) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x * time->dt;
        pos->y += vel->y * time->dt;
    }
}

// Application: Tie it together
Application{}
    .add_systems<movement_system>(Schedule::UPDATE)
    .run();
```

This separation of data and logic provides:
- **Flexibility**: Easy to add, remove, or modify behavior
- **Performance**: Data-oriented design optimized for modern CPUs
- **Maintainability**: Clear separation of concerns
- **Testability**: Systems can be tested in isolation

## Next Steps

Start with [Entities](./entities.md) to understand the foundation of the ECS pattern.
