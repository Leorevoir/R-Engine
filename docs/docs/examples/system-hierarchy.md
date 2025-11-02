---
sidebar_position: 2
---

# System Hierarchy Example

Demonstrates parent-child entity relationships using hierarchies for transform propagation.

## Overview

This example shows how to create and manage hierarchical entity structures. A central "parent" entity rotates, and its "child" entities orbit around it, inheriting its transformation. This is useful for:

- Transform propagation (like a planet orbiting a sun)
- Scene graphs
- UI layouts
- Nested game objects

## Components

```cpp
// Local transform relative to the parent
struct Transform3d {
    Vec3f position;
    Vec3f rotation; // Euler angles in radians
    Vec3f scale = {1.0f, 1.0f, 1.0f};
};

// Global transform in world space, calculated by the engine
struct GlobalTransform3d : public Transform3d {};
```

## Creating the Hierarchy

We spawn a parent entity and attach children to it using `with_children`.

```cpp
void spawn_hierarchy(ecs::Commands& commands) {
    // Create the parent entity (the "sun")
    commands.spawn(
        Transform3d{ .position = {400, 300, 0} },
        Circle{ .radius = 20.0f, .color = Color::Yellow }
    )
    .with_children([](ecs::ChildBuilder& builder) {
        // Child 1 (a "planet")
        builder.spawn(
            Transform3d{ .position = {100, 0, 0}, .scale = {0.5f, 0.5f, 0.5f} },
            Circle{ .radius = 15.0f, .color = Color::Blue }
        );

        // Child 2 (another "planet")
        builder.spawn(
            Transform3d{ .position = {-150, 0, 0}, .scale = {0.3f, 0.3f, 0.3f} },
            Circle{ .radius = 12.0f, .color = Color::Green }
        );
    });
}
```

## Transform Propagation System

The engine's `TransformPlugin` (included in `DefaultPlugins`) automatically handles transform propagation. It runs a system that calculates the `GlobalTransform3d` of every entity based on its local `Transform3d` and its parent's `GlobalTransform3d`. This happens every frame before rendering.

## Rotation System

This system finds the root entity (the one without a `Parent` component) and rotates its local transform. The transform propagation system will then ensure this rotation is inherited by the children.

```cpp
void rotate_parent(
    ecs::Query<ecs::Mut<Transform3d>, ecs::Without<ecs::Parent>> query,
    ecs::Res<core::FrameTime> time
) {
    for (auto [transform, _] : query) {
        // Rotate around the Z-axis
        transform.ptr->rotation.z += 1.0f * time.ptr->delta_time;
    }
}
```

## Rendering System

This system simply draws a circle for every entity at its final `GlobalTransform3d` position.

```cpp
void render_hierarchy(ecs::Query<ecs::Ref<GlobalTransform3d>, ecs::Ref<Circle>> query) {
    for (auto [transform, circle] : query) {
        DrawCircle(
            transform.ptr->position.x,
            transform.ptr->position.y,
            circle.ptr->radius,
            circle.ptr->color
        );
    }
}
```

## Application Setup

```cpp
int main() {
    Application{}
        .add_plugins(DefaultPlugins{}) // Includes the essential TransformPlugin
        .add_systems<spawn_hierarchy>(Schedule::STARTUP)
        .add_systems<rotate_parent>(Schedule::UPDATE)
        .add_systems<render_hierarchy>(Schedule::RENDER_2D)
        .run();

    return 0;
}
```

## Key Concepts

1.  **Hierarchy Components**: `ecs::Parent` and `ecs::Children` components link entities.
2.  **Local vs Global**: `Transform3d` is for local modifications, `GlobalTransform3d` is the computed result.
3.  **Automatic Propagation**: The engine handles the math to ensure children inherit parent transformations.
4.  **System Ordering**: The `TransformPlugin` ensures propagation happens after updates but before rendering.

## Next Steps

- Learn more about [Hierarchies](../advanced/hierarchies.md)
- Try [Event Communication](./event-communication.md)
