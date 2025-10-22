---
sidebar_position: 2
---

# Hierarchies

Hierarchies enable parent-child relationships between entities, useful for transforms, UI layouts, and scene graphs.

## Creating Hierarchies

### With Children

Spawn entities with children:

```cpp
void setup(Commands& commands) {
    // Create parent
    Entity parent = commands.spawn(Transform{}).id();
    
    // Add children
    commands.entity(parent).with_children([&](Commands& child_commands) {
        child_commands.spawn(Transform{});
        child_commands.spawn(Transform{});
    });
}
```

### Set Parent

Set parent after creation:

```cpp
Entity parent = commands.spawn(Transform{}).id();
Entity child = commands.spawn(Transform{}).id();

commands.entity(child).set_parent(parent);
```

## Traversing Hierarchies

### Get Children

```cpp
void system(Query<Entity, Ref<Children>> query) {
    for (auto [entity, children] : query) {
        for (Entity child : children->entities) {
            // Process child
        }
    }
}
```

### Get Parent

```cpp
void system(Query<Entity, Ref<Parent>> query) {
    for (auto [entity, parent] : query) {
        Entity parent_entity = parent->entity;
        // Process parent
    }
}
```

## Transform Hierarchy

Common use case - transform propagation:

```cpp
struct Transform {
    Vec3 position;
    Vec3 rotation;
    Vec3 scale = {1, 1, 1};
};

struct GlobalTransform {
    Mat4 matrix;
};

void transform_propagation(
    Query<Mut<GlobalTransform>, Ref<Transform>, Ref<Parent>> query,
    Query<Ref<GlobalTransform>> parent_query
) {
    for (auto [global, local, parent] : query) {
        auto parent_global = parent_query.get<Ref<GlobalTransform>>(parent->entity);
        if (parent_global) {
            global->matrix = parent_global->matrix * local->to_matrix();
        }
    }
}
```

## Removing from Hierarchy

```cpp
// Remove child from parent
commands.entity(child).remove_parent();

// Despawn with children
commands.entity(parent).despawn_recursive();
```

## Best Practices

- Use hierarchies for logical grouping
- Propagate transforms from parent to child
- Be careful with recursive operations

## Next Steps

- Learn about [Run Conditions](./run-conditions.md)
- See [Examples](../examples/system-hierarchy.md)
