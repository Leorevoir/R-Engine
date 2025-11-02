---
sidebar_position: 3
---

# EntityCommands API

`ecs::EntityCommands` provides a builder pattern for modifying a specific entity.

## Methods

### insert()

```cpp
EntityCommands& insert(T component) noexcept;
```

Schedules a component to be added to the entity.

**Returns**: Self for chaining.

### remove()

```cpp
EntityCommands& remove<T>() noexcept;
```

Schedules a component to be removed from the entity.

**Returns**: Self for chaining.

### with_children()

```cpp
EntityCommands& with_children(FuncT&& func) noexcept;
```

Spawns child entities for this entity. The provided function receives a `ChildBuilder`.

**Returns**: Self for chaining.

### id()

```cpp
Entity id() const noexcept;
```

Returns the entity's ID. If the entity was just spawned, this is a temporary placeholder ID valid only within the current command buffer cycle.

## Example Usage

```cpp
void system(ecs::Commands& commands) {
    // Create and configure a new entity
    commands.spawn()
        .insert(Position{0, 0})
        .insert(Velocity{1, 0})
        .insert(Player{});

    // Modify an existing entity
    ecs::Entity e = /* ... */;
    commands.entity(e)
        .insert(Health{100})
        .remove<Frozen>();

    // Create a hierarchy
    commands.spawn(Transform3d{})
        .with_children([](ecs::ChildBuilder& builder) {
            builder.spawn(Transform3d{ .position = {1, 0, 0} });
            builder.spawn(Transform3d{ .position = {-1, 0, 0} });
        });
}
```

## Method Chaining

Most methods return `EntityCommands&` for convenient chaining:

```cpp
commands.entity(e)
    .insert(ComponentA{})
    .insert(ComponentB{})
    .remove<ComponentC>();
```

## See Also

- [Commands](./commands.md) - The main interface for all world modifications.
- [Hierarchies](../advanced/hierarchies.md) - More on parent-child relationships.
