---
sidebar_position: 4
---

# Query API

`ecs::Query` enables type-safe iteration over entities with a specific set of components.

## Template Parameters

```cpp
ecs::Query<Wrappers...>
```

Wrappers define the components to access and the filters to apply:

- `Ref<T>`: Read-only access to component `T`.
- `Mut<T>`: Mutable (read/write) access to component `T`.
- `With<T>`: Filter to include only entities that _have_ component `T`.
- `Without<T>`: Filter to exclude entities that _have_ component `T`.
- `Optional<T>`: Optional read-only access to component `T`. The resulting pointer will be `nullptr` if the component is missing.

## Iteration

### Range-Based For Loop

The most common way to use a query is with a range-based for loop.

```cpp
void system(ecs::Query<ecs::Mut<Position>, ecs::Ref<Velocity>> query) {
    for (auto [pos, vel] : query) {
        pos.ptr->x += vel.ptr->x;
    }
}
```

### Accessing the Entity ID

To get the entity ID, use an iterator and its `.entity()` method.

```cpp
void system(ecs::Query<ecs::Ref<Position>> query) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        auto [pos] = *it;
        ecs::Entity entity_id = it.entity();
        std::cout << "Entity " << entity_id << "\n";
    }
}
```

## Filters

### With\<T\>

Only include entities that have component `T`:

```cpp
// This query will only iterate over entities that have a Player component.
ecs::Query<ecs::Ref<Position>, ecs::With<Player>> query;
```

### Without\<T\>

Exclude entities that have component `T`:

```cpp
// This query will only iterate over entities that do NOT have a Dead component.
ecs::Query<ecs::Ref<Position>, ecs::Without<Dead>> query;
```

### Combined Filters

You can combine multiple filters.

```cpp
// Get mutable access to the Position of entities that are Players,
// are not Frozen, and are not Dead.
ecs::Query<
    ecs::Mut<Position>,
    ecs::With<Player>,
    ecs::Without<Frozen>,
    ecs::Without<Dead>
> query;
```

## Methods

### size()

```cpp
u64 size() const;
```

Returns the number of entities that currently match the query.

## Accessing Component Pointers

When you iterate, you get wrappers (`Mut<T>`, `Ref<T>`) that contain a `ptr` to the component data.

### Ref\<T\> - Read-Only

```cpp
ecs::Query<ecs::Ref<Position>> query;
for (auto [pos] : query) {
    float x = pos.ptr->x;  // ✓ Read is OK
    // pos.ptr->x = 0;     // ✗ Compile Error: ptr is const
}
```

### Mut\<T\> - Mutable

```cpp
ecs::Query<ecs::Mut<Position>> query;
for (auto [pos] : query) {
    pos.ptr->x = 0;  // ✓ Write is OK
}
```

## Performance Tips

- Use `Ref<T>` instead of `Mut<T>` whenever you don't need to modify the data. This allows the scheduler to potentially run more systems in parallel.
- Use `With<T>` and `Without<T>` filters to narrow down the set of entities your system needs to process.
- Avoid querying for components your system doesn't actually use.

## See Also

- [Queries Guide](../core-concepts/queries.md)
- [Components](../core-concepts/components.md)
