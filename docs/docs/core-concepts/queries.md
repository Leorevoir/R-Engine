---
sidebar_position: 5
---

# Queries

Queries are the primary way for systems to access and iterate over entities that have a specific set of components. They provide safe, efficient, and type-safe access to component data.

## Basic Query

A query is defined by the component wrappers passed as its template arguments. This example queries for all entities that have both a `Position` and a `Velocity` component.

```cpp
void movement_system(ecs::Query<ecs::Mut<Position>, ecs::Ref<Velocity>> query) {
    for (auto [pos, vel] : query) {
        // Process entities with both Position AND Velocity
        pos.ptr->x += vel.ptr->x;
    }
}
```

## Access Modes (Wrappers)

### `Ref<T>` - Read-Only Access

Use `Ref<T>` when you only need to read a component's data. This is the preferred default as it allows the scheduler more opportunities for parallelism.

```cpp
ecs::Query<ecs::Ref<Position>> query;
// Provides read-only access to Position.
```

### `Mut<T>` - Mutable Access

Use `Mut<T>` when you need to modify a component's data.

```cpp
ecs::Query<ecs::Mut<Position>> query;
// Provides read/write access to Position.
```

## Query Filters

Filters allow you to narrow down which entities a query will match without accessing the component data.

### `With<T>` - Must Have Component

Selects entities that have the component `T`.

```cpp
// Gets the Position of entities that also have a Player component.
ecs::Query<ecs::Ref<Position>, ecs::With<Player>> query;
```

### `Without<T>` - Must Not Have Component

Selects entities that do **not** have the component `T`.

```cpp
// Gets the Position of entities that do NOT have a Dead component.
ecs::Query<ecs::Ref<Position>, ecs::Without<Dead>> query;
```

### `Optional<T>` - May Have Component

Provides optional read-only access to a component. The wrapper's pointer will be `nullptr` if the entity does not have the component.

```cpp
void system(ecs::Query<ecs::Ref<Position>, ecs::Optional<Health>> query) {
    for (auto [pos, health_opt] : query) {
        if (health_opt.ptr) {
            // This entity has a Health component.
        } else {
            // This entity does not.
        }
    }
}
```

## Getting the Entity ID

To get the ID of the entity you are currently iterating over, use the iterator's `.entity()` method.

```cpp
void system(ecs::Query<ecs::Ref<Position>> query) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        // Get the component tuple from the iterator
        auto [pos] = *it;

        // Get the entity ID from the iterator
        ecs::Entity entity_id = it.entity();

        std::cout << "Entity " << entity_id
                  << " is at (" << pos.ptr->x << ", " << pos.ptr->y << ")\n";
    }
}
```

## Best Practices

- Always use `Ref<T>` by default and only switch to `Mut<T>` when modification is necessary.
- Use filters (`With<T>`, `Without<T>`) to be as specific as possible, reducing the number of entities your system has to iterate over.
- Only include components in the query that your system actually needs to access.

## Next Steps

- Learn about [Commands](./commands.md) for modifying entities.
- See [Examples](../examples/index.md) for practical query usage.
