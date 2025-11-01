---
sidebar_position: 1
---

# Scene API

The Scene class manages the ECS world - entities, components, resources, and systems.

## Methods

### spawn()

```cpp
Entity spawn()
Entity spawn(Components... components)
```

Creates a new entity.

**Returns**: Entity ID

**Example**:
```cpp
Entity e = scene.spawn();
Entity player = scene.spawn(Position{}, Health{100});
```

### despawn()

```cpp
void despawn(Entity entity)
```

Removes an entity and all its components.

### insert_resource()

```cpp
void insert_resource(T resource)
```

Adds or replaces a global resource.

### remove_resource()

```cpp
void remove_resource<T>()
```

Removes a global resource.

### get_resource()

```cpp
T* get_resource<T>()
const T* get_resource<T>() const
```

Gets a resource if it exists.

**Returns**: Pointer to resource, or nullptr

## Example Usage

```cpp
Scene scene;

// Spawn entities
Entity player = scene.spawn(Player{}, Position{0, 0});

// Add resources
scene.insert_resource(GameConfig{});

// Access resources
if (auto* config = scene.get_resource<GameConfig>()) {
    // Use config
}

// Despawn
scene.despawn(player);
```

## See Also

- [Commands](./commands.md) - Deferred modifications
- [Query](./query.md) - Entity iteration
