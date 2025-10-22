---
sidebar_position: 5
---

# Queries

Queries allow systems to iterate over entities that match specific criteria. They provide type-safe access to components.

## Basic Query

```cpp
void system(Query<Ref<Position>, Ref<Velocity>> query) {
    for (auto [pos, vel] : query) {
        // Process entities with Position AND Velocity
    }
}
```

## Access Modes

### Ref\<T\> - Read-Only

```cpp
Query<Ref<Position>> query
// Read-only access to Position
```

### Mut\<T\> - Mutable

```cpp
Query<Mut<Position>> query
// Mutable access to Position
```

## Query Filters

### With\<T\> - Has Component

```cpp
Query<Ref<Position>, With<Player>> query
// Entities that have Position AND Player components
```

### Without\<T\> - Lacks Component

```cpp
Query<Ref<Position>, Without<Dead>> query
// Entities with Position but NOT Dead component
```

### Combined Filters

```cpp
Query<Mut<Position>, With<Player>, Without<Frozen>> query
// Player entities that are not frozen
```

## Getting Entity ID

```cpp
void system(Query<Entity, Ref<Position>> query) {
    for (auto [entity, pos] : query) {
        std::cout << "Entity " << entity.id() << "\n";
    }
}
```

## Query Iteration

### Range-Based For Loop

```cpp
for (auto [pos, vel] : query) {
    pos->x += vel->x;
}
```

### Manual Iteration

```cpp
for (auto it = query.begin(); it != query.end(); ++it) {
    auto [pos, vel] = *it;
    // Process...
}
```

## Best Practices

- Use `Ref<T>` by default, `Mut<T>` only when needed
- Use filters to reduce iterations
- Access entity ID when you need to reference entities

## Next Steps

- Learn about [Commands](./commands.md)
- See [Examples](../examples/index.md)
