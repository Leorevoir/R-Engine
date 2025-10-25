---
sidebar_position: 1
---

# Entities

Entities are unique identifiers that represent objects in your application. They are lightweight and tie components together.

## What is an Entity?

An entity is simply a unique ID (typically a 32-bit or 64-bit integer). It has no data or behavior on its own — it's just a handle that groups components together.

```cpp
// An entity is just an ID
Entity player = Entity{42};
```

## Creating Entities

Entities are created using the `Commands` system parameter:

```cpp
void spawn_system(Commands& commands) {
    // Spawn an empty entity
    Entity empty = commands.spawn().id();
    
    // Spawn an entity with components
    Entity player = commands.spawn(
        Position{0.0f, 0.0f},
        Velocity{1.0f, 0.0f},
        Health{100}
    ).id();
}
```

## Entity Identity

Each entity has a unique identifier that remains valid until the entity is despawned:

```cpp
void system(Commands& commands) {
    Entity e1 = commands.spawn().id();
    Entity e2 = commands.spawn().id();
    
    // e1 != e2 (always unique)
}
```

:::caution Entity Lifecycle
Once an entity is despawned, its ID may be reused. Never store entity IDs across frames unless you're certain the entity still exists.
:::

## Despawning Entities

Remove entities using `despawn`:

```cpp
void cleanup_system(
    Query<Entity, With<Dead>> query,
    Commands& commands
) {
    for (auto [entity, _] : query) {
        commands.entity(entity).despawn();
    }
}
```

## Entity Relationships

Entities can have parent-child relationships using the hierarchy system:

```cpp
void spawn_hierarchy(Commands& commands) {
    // Create parent
    Entity parent = commands.spawn(Transform{}).id();
    
    // Create children
    commands.entity(parent).with_children([&](Commands& child_builder) {
        child_builder.spawn(Transform{});
        child_builder.spawn(Transform{});
    });
}
```

[Learn more about Hierarchies →](../advanced/hierarchies.md)

## Querying Entities

Get entities in queries using the `Entity` type:

```cpp
void system(Query<Entity, Ref<Position>> query) {
    for (auto [entity, pos] : query) {
        std::cout << "Entity " << entity.id() 
                  << " at (" << pos->x << ", " << pos->y << ")\n";
    }
}
```

## Best Practices

### ✅ Do

- Use entities as lightweight handles
- Despawn entities when no longer needed
- Use marker components to categorize entities

```cpp
struct Player {};
struct Enemy {};
struct Bullet {};

// Easy to query specific entity types
void player_system(Query<Ref<Position>, With<Player>> query) {
    // Only processes player entities
}
```

### ❌ Don't

- Don't store entity IDs for long periods
- Don't use entities as data containers (use components instead)
- Don't manually manage entity IDs

## Example: Entity Pool

```cpp
// Spawn multiple entities
void spawn_enemies(Commands& commands) {
    for (int i = 0; i < 10; i++) {
        commands.spawn(
            Enemy{},
            Position{rand() % 800, rand() % 600},
            Health{50}
        );
    }
}

// Process all enemies
void enemy_ai_system(
    Query<Entity, Mut<Position>, Ref<Health>, With<Enemy>> query,
    Commands& commands
) {
    for (auto [entity, pos, health, _] : query) {
        // AI logic...
        
        // Despawn if dead
        if (health->current <= 0) {
            commands.entity(entity).despawn();
        }
    }
}
```

## Next Steps

- Learn about [Components](./components.md) to add data to entities
- See how [Systems](./systems.md) operate on entities
- Explore [Commands](./commands.md) for entity manipulation
