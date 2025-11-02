---
sidebar_position: 1
---

# Entities

Entities are unique identifiers that represent objects in your application. They are lightweight and serve as a handle to group a set of components together.

## What is an Entity?

An entity is simply a unique ID (an unsigned 32-bit integer, aliased as `ecs::Entity`). It has no data or behavior on its own. Its sole purpose is to associate a collection of components.

```cpp
// An entity is just an ID
ecs::Entity player_id = 42;
```

## Creating Entities

Entities are created using the `ecs::Commands` system parameter.

```cpp
void spawn_system(ecs::Commands& commands) {
    // Spawn an empty entity
    ecs::Entity empty_entity = commands.spawn().id();

    // Spawn an entity with some initial components
    ecs::Entity player = commands.spawn(
        Position{0.0f, 0.0f},
        Velocity{1.0f, 0.0f},
        Health{100}
    ).id();
}
```

## Entity Identity

Each entity has a unique identifier that remains valid until the entity is despawned.

```cpp
void system(ecs::Commands& commands) {
    ecs::Entity e1 = commands.spawn().id();
    ecs::Entity e2 = commands.spawn().id();

    // e1 will not be equal to e2
}
```

:::caution Entity Lifecycle
Once an entity is despawned, its ID may eventually be reused by the engine. Avoid storing raw `Entity` IDs for long periods. If you need a stable reference, consider creating a resource that holds the entity ID.
:::

## Despawning Entities

Remove entities and all their components using `commands.despawn()`.

```cpp
void cleanup_system(
    ecs::Query<ecs::Entity, ecs::With<Dead>> query,
    ecs::Commands& commands
) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        commands.despawn(it.entity());
    }
}
```

## Entity Relationships (Hierarchies)

Entities can have parent-child relationships, which is essential for transform propagation and scene graphs.

```cpp
void spawn_hierarchy(ecs::Commands& commands) {
    // Create a parent entity
    commands.spawn(Transform3d{ .position = {400, 300, 0} })
        // Attach children to the parent
        .with_children([](ecs::ChildBuilder& builder) {
            builder.spawn(Transform3d{ .position = {50, 0, 0} });
            builder.spawn(Transform3d{ .position = {-50, 0, 0} });
        });
}
```

[Learn more about Hierarchies →](../advanced/hierarchies.md)

## Querying for Entities

To get the ID of an entity you are processing in a query, use the iterator's `.entity()` method.

```cpp
void system(ecs::Query<ecs::Ref<Position>> query) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        auto [pos] = *it;
        ecs::Entity entity = it.entity();

        std::cout << "Entity " << entity
                  << " is at (" << pos.ptr->x << ", " << pos.ptr->y << ")\n";
    }
}
```

## Best Practices

### ✅ Do

- Think of entities as simple, lightweight handles.
- Despawn entities when they are no longer needed to free up resources.
- Use marker components to categorize entities (e.g., `struct Player {};`).

```cpp
struct Player {};
struct Enemy {};

// This makes it easy to query for specific types of entities.
void player_system(ecs::Query<ecs::Ref<Position>, ecs::With<Player>> query) {
    // This system will only process entities that have a Player component.
}
```

### ❌ Don't

- Don't store entity IDs long-term without a strategy to handle despawning.
- Don't try to embed data or logic into the entity itself; that's what components and systems are for.
- Don't manage entity IDs manually; always let `Commands` handle creation.

## Next Steps

- Learn about [Components](./components.md) to add data to entities.
- See how [Systems](./systems.md) operate on entities.
- Explore [Commands](./commands.md) for entity manipulation.
