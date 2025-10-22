---
sidebar_position: 3
---

# EntityCommands API

Commands for modifying a specific entity.

## Methods

### insert()

```cpp
EntityCommands& insert(Component component)
```

Add a component to the entity.

**Returns**: Self for chaining

### remove()

```cpp
EntityCommands& remove<T>()
```

Remove a component from the entity.

**Returns**: Self for chaining

### despawn()

```cpp
void despawn()
```

Queue entity for despawning.

### set_parent()

```cpp
EntityCommands& set_parent(Entity parent)
```

Set parent entity (creates hierarchy).

### with_children()

```cpp
EntityCommands& with_children(Function fn)
```

Spawn children for this entity.

### id()

```cpp
Entity id() const
```

Get the entity ID.

## Example Usage

```cpp
void system(Commands& commands) {
    // Create and configure entity
    commands.spawn()
        .insert(Position{0, 0})
        .insert(Velocity{1, 0})
        .insert(Player{});
    
    // Modify existing entity
    Entity e = /* ... */;
    commands.entity(e)
        .insert(Health{100})
        .remove<Frozen>();
    
    // Create hierarchy
    Entity parent = commands.spawn(Transform{}).id();
    commands.entity(parent).with_children([&](Commands& child) {
        child.spawn(Transform{});
    });
}
```

## Method Chaining

Most methods return `EntityCommands&` for convenient chaining:

```cpp
commands.entity(e)
    .insert(A{})
    .insert(B{})
    .remove<C>()
    .set_parent(parent);
```

## See Also

- [Commands](./commands.md) - General commands
- [Hierarchies](../advanced/hierarchies.md) - Parent-child relationships
