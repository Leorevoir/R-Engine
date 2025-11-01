---
sidebar_position: 2
---

# System Hierarchy Example

Demonstrates parent-child entity relationships using hierarchies.

## Overview

This example shows how to create and manage hierarchical entity structures, useful for:
- Transform propagation
- Scene graphs
- UI layouts
- Nested game objects

## Setup

```cpp
struct Transform {
    Vec2f position;
    float rotation;
    float scale = 1.0f;
};

struct GlobalTransform {
    Vec2f position;
    float rotation;
    float scale = 1.0f;
};
```

## Creating Hierarchy

```cpp
void spawn_hierarchy(Commands& commands) {
    // Create parent entity
    Entity parent = commands.spawn(
        Transform{Vec2f{400, 300}, 0.0f, 1.0f},
        GlobalTransform{}
    ).id();
    
    // Add children
    commands.entity(parent).with_children([&](Commands& child_cmds) {
        // Child 1 - offset to the right
        child_cmds.spawn(
            Transform{Vec2f{50, 0}, 0.0f, 0.5f},
            GlobalTransform{}
        );
        
        // Child 2 - offset to the left
        child_cmds.spawn(
            Transform{Vec2f{-50, 0}, 0.0f, 0.5f},
            GlobalTransform{}
        );
    });
}
```

## Transform Propagation

```cpp
void propagate_transforms(
    Query<Mut<GlobalTransform>, Ref<Transform>, Without<Parent>> root_query,
    Query<Mut<GlobalTransform>, Ref<Transform>, Ref<Parent>> child_query,
    Query<Ref<GlobalTransform>> parent_transform_query
) {
    // Update root entities (no parent)
    for (auto [global, local, _] : root_query) {
        global.ptr->position = local.ptr->position;
        global.ptr->rotation = local.ptr->rotation;
        global.ptr->scale = local.ptr->scale;
    }
    
    // Update children (propagate from parent)
    for (auto [global, local, parent] : child_query) {
        auto parent_global = parent_transform_query
            .get<Ref<GlobalTransform>>(parent.ptr->entity);
        
        if (parent_global) {
            // Combine parent and local transforms
            global.ptr->position = parent_global.ptr->position + 
                                  local.ptr->position * parent_global.ptr->scale;
            global.ptr->rotation = parent_global.ptr->rotation + 
                                  local.ptr->rotation;
            global.ptr->scale = parent_global.ptr->scale * 
                               local.ptr->scale;
        }
    }
}
```

## Rotation System

```cpp
void rotate_parent(
    Query<Mut<Transform>, Without<Parent>> query,
    Res<DeltaTime> time
) {
    for (auto [transform, _] : query) {
        transform.ptr->rotation += 1.0f * time.ptr->dt;
    }
}
```

## Rendering

```cpp
void render_hierarchy(Query<Ref<GlobalTransform>> query) {
    for (auto [transform] : query) {
        draw_circle_at(
            transform.ptr->position,
            10.0f * transform.ptr->scale,
            Color::Blue
        );
    }
}
```

## Application

```cpp
int main() {
    Application{}
        .insert_resource(DeltaTime{})
        .add_systems<spawn_hierarchy>(Schedule::STARTUP)
        .add_systems<
            rotate_parent,
            propagate_transforms
        >(Schedule::UPDATE)
        .after<rotate_parent>()  // Propagate after rotation
        .add_systems<render_hierarchy>(Schedule::RENDER_2D)
        .run();
    
    return 0;
}
```

## Key Concepts

1. **Hierarchy Components**: `Parent` and `Children` components link entities
2. **Local vs Global**: Separate transforms for local and world-space
3. **Transform Propagation**: Children inherit parent transformations
4. **System Ordering**: Propagation must run after parent updates

## Running

```bash
./r-engine__system_hierarchy
```

## Next Steps

- Learn more about [Hierarchies](../advanced/hierarchies.md)
- Try [Event Communication](./event-communication.md)
