---
sidebar_position: 1
---

# Scene API

The `ecs::Scene` class is the core container in the ECS, managing all entities, components, and resources.

:::caution Direct Modification
While `Scene` provides the low-level API to manipulate the world, systems should **almost always** use `ecs::Commands` for modifications. Direct calls to `Scene` methods like `add_component` or `destroy_entity` from a system can lead to crashes by invalidating active queries.
:::

## Methods for Systems

The main way systems interact with the scene is by requesting resources.

### get_resource_ptr()

```cpp
template<typename T>
T* get_resource_ptr() noexcept;
```

Gets a raw pointer to a global resource if it exists.

**Returns**: A pointer to the resource, or `nullptr` if the resource does not exist.

**Example**:

```cpp
// In a system, you would typically use Res<T> or ResMut<T>
// but this shows the underlying access.
void some_function(ecs::Scene& scene) {
    if (auto* config = scene.get_resource_ptr<GameConfig>()) {
        // Use the config
    }
}
```

## Low-Level Methods (For Internal Use)

These methods are used internally by the engine and `Commands` to apply changes. Avoid calling them directly from systems.

### create_entity() / destroy_entity()

```cpp
Entity create_entity();
void destroy_entity(Entity e) noexcept;
```

Creates or destroys an entity. **Use `commands.spawn()` or `commands.despawn()` instead.**

### add_component() / remove_component()

````cpp
template<typename T> void add_component(Entity e, T comp);
template<typename T> void remove_component(Entity e);```
Adds or removes a component from an entity. **Use `commands.entity(e).insert()` or `.remove()` instead.**

### insert_resource() / remove_resource()

```cpp
template<typename T> void insert_resource(T&& r) noexcept;
template<typename T> void remove_resource() noexcept;
````

Adds or removes a global resource. **Use `commands.insert_resource()` or `.remove_resource()` instead.**

## Example: Correct Usage in a System

```cpp
// Correct way for a system to interact with the world
void my_system(
    ecs::Commands& commands,                 // For modifications
    ecs::Res<GameConfig> config,             // For reading resources
    ecs::Query<ecs::Mut<Position>> query     // For accessing components
) {
    // Spawn a new entity
    commands.spawn(ScoreText{});

    // Modify entities safely
    for (auto [pos] : query) {
        if (pos.ptr->x > config.ptr->world_width) {
            // This is safe because it's a deferred command
            commands.despawn(it.entity());
        }
    }
}
```

## See Also

- [Commands](./commands.md) - The safe, primary way to modify the world.
- [Query](./query.md) - The primary way to read component data.
