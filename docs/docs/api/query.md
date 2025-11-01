---
sidebar_position: 4
---

# Query API

Type-safe entity iteration with component access.

## Template Parameters

```cpp
Query<Wrappers...>
```

Wrappers can be:
- `Ref<T>` - Read-only component access
- `Mut<T>` - Mutable component access
- `With<T>` - Filter: entity must have T
- `Without<T>` - Filter: entity must not have T
- `Entity` - Get entity ID

## Iteration

### Range-Based For

```cpp
Query<Mut<Position>, Ref<Velocity>> query;

for (auto [pos, vel] : query) {
    pos.ptr->x += vel.ptr->x;
}
```

### With Entity ID

```cpp
Query<Entity, Ref<Position>> query;

for (auto [entity, pos] : query) {
    std::cout << "Entity " << entity.id() << "\n";
}
```

## Filters

### With\<T\>

Only entities that have component T:

```cpp
Query<Ref<Position>, With<Player>> query;
// Only processes player entities
```

### Without\<T\>

Only entities that lack component T:

```cpp
Query<Ref<Position>, Without<Dead>> query;
// Only processes living entities
```

### Combined

```cpp
Query<
    Mut<Position>,
    With<Player>,
    Without<Frozen>,
    Without<Dead>
> query;
// Active players only
```

## Methods

### get()

```cpp
template<typename Wrapper>
std::optional<Wrapper> get(Entity e)
```

Get component for specific entity.

**Example**:
```cpp
if (auto pos = query.get<Ref<Position>>(entity)) {
    // Use pos
}
```

### is_empty()

```cpp
bool is_empty() const
```

Check if query matches any entities.

## Access Modes

### Ref\<T\> - Read-Only

```cpp
Query<Ref<Position>> query
for (auto [pos] : query) {
    float x = pos.ptr->x;  // ✓ Read
    // pos.ptr->x = 0;     // ✗ Error: const
}
```

### Mut\<T\> - Mutable

```cpp
Query<Mut<Position>> query
for (auto [pos] : query) {
    pos.ptr->x = 0;  // ✓ Write allowed
}
```

## Performance Tips

- Use `Ref<T>` instead of `Mut<T>` when you don't need to modify
- Use filters to reduce iterations
- Avoid querying for unnecessary components

## See Also

- [Queries Guide](../core-concepts/queries.md)
- [Components](../core-concepts/components.md)
