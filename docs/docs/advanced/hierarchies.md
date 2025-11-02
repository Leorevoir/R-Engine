---
sidebar_position: 2
---

# Hierarchies

Hierarchies enable parent-child relationships between entities, useful for transforms, UI layouts, and scene graphs.

## Creating Hierarchies

### With Children

The primary way to create hierarchies is to spawn entities with children using the `with_children` builder on `EntityCommands`.

```cpp
void setup(ecs::Commands& commands) {
    // Create a parent entity
    commands.spawn(Transform3d{})
        // Use with_children to add children to the parent
        .with_children([](ecs::ChildBuilder &builder) {
            // The builder spawns entities that are automatically parented
            builder.spawn(Transform3d{ .position = { 1.0f, 0.0f, 0.0f } });
            builder.spawn(Transform3d{ .position = { -1.0f, 0.0f, 0.0f } });
        });
}
```

This automatically adds a `Children` component to the parent and a `Parent` component to each child.

## Traversing Hierarchies

### Get Children

The `Children` component contains a vector of child entities.

```cpp
void system(ecs::Query<ecs::Ref<ecs::Children>> query) {
    for (auto [children] : query) {
        for (ecs::Entity child : children.ptr->entities) {
            // Process child entity
        }
    }
}
```

### Get Parent

The `Parent` component stores the parent's entity ID.

````cpp
void system(ecs::Query<ecs::Ref<ecs::Parent>> query) {
    for (auto [parent] : query) {
        ecs::Entity parent_entity = parent.ptr->entity;
        // Process parent entity
    }
}```

## Transform Hierarchy

The most common use case is transform propagation, which the `TransformPlugin` handles automatically.

-   **`Transform3d`**: Represents the entity's local position, rotation, and scale relative to its parent.
-   **`GlobalTransform3d`**: Represents the entity's final position, rotation, and scale in world space.

The `transform_propagate_system` runs automatically in the `BEFORE_RENDER_3D` schedule, calculating `GlobalTransform3d` for all entities based on their `Transform3d` and their parent's `GlobalTransform3d`.

```cpp
// The engine does this for you!
void transform_propagation_concept(
    ecs::Query<ecs::Ref<Transform3d>, ecs::Mut<GlobalTransform3d>, ecs::Ref<ecs::Parent>> query,
    ecs::Query<ecs::Ref<GlobalTransform3d>> parent_query
) {
    for (auto [local, global, parent] : query) {
        if (auto* parent_global = parent_query.get_component_ptr<GlobalTransform3d>(parent.ptr->entity)) {
            // This logic is handled by GlobalTransform3d::from_local_and_parent
            *global.ptr = GlobalTransform3d::from_local_and_parent(*local.ptr, *parent_global);
        }
    }
}
````

## Removing from Hierarchy

Despawning an entity with `commands.despawn(entity)` will also despawn all of its descendants recursively.

```cpp
// This will despawn the parent and all its children.
commands.despawn(parent_entity);
```

## Best Practices

- Use hierarchies for logical grouping and transform propagation.
- Modify the `Transform3d` component; let the engine calculate `GlobalTransform3d`.
- Be mindful of recursive operations when traversing hierarchies manually.

## Next Steps

- Learn about [Run Conditions](./run-conditions.md)
- See [Examples](../examples/system-hierarchy.md)
