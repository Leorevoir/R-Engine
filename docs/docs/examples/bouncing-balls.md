---
sidebar_position: 1
---

# Bouncing Balls Example

A simple physics simulation demonstrating entities, components, and systems.

## Overview

This example creates multiple ball entities that bounce around the screen, demonstrating:

- Entity spawning
- Component-based data
- System logic for movement and collision
- Query iteration
- Resource usage for frame time

## Components

```cpp
struct Position {
    Vec2f value;
};

struct Velocity {
    Vec2f value;
};

struct Circle {
    float radius;
    Color color;
};
```

## Systems

### Spawn System

Creates ball entities once at startup.

```cpp
void spawn_system(ecs::Commands& commands) {
    for (int i = 0; i < 10; i++) {
        commands.spawn(
            Position{Vec2f{rand() % 800, rand() % 600}},
            Velocity{Vec2f{rand_float(-100, 100), rand_float(-100, 100)}},
            Circle{20.0f, random_color()}
        );
    }
}
```

### Movement System

Updates positions based on velocity each frame.

```cpp
void movement_system(
    ecs::Query<ecs::Mut<Position>, ecs::Ref<Velocity>> query,
    ecs::Res<core::FrameTime> time
) {
    for (auto [pos, vel] : query) {
        pos.ptr->value += vel.ptr->value * time.ptr->delta_time;
    }
}
```

### Bounce System

Reverses velocity when balls hit the screen edges.

```cpp
void bounce_system(
    ecs::Query<ecs::Ref<Position>, ecs::Mut<Velocity>, ecs::Ref<Circle>> query
) {
    for (auto [pos, vel, circle] : query) {
        // Left/right walls
        if ((pos.ptr->value.x - circle.ptr->radius < 0 && vel.ptr->value.x < 0) ||
            (pos.ptr->value.x + circle.ptr->radius > 800 && vel.ptr->value.x > 0)) {
            vel.ptr->value.x *= -1;
        }

        // Top/bottom walls
        if ((pos.ptr->value.y - circle.ptr->radius < 0 && vel.ptr->value.y < 0) ||
            (pos.ptr->value.y + circle.ptr->radius > 600 && vel.ptr->value.y > 0)) {
            vel.ptr->value.y *= -1;
        }
    }
}
```

### Render System

Draws the balls.

```cpp
void render_system(
    ecs::Query<ecs::Ref<Position>, ecs::Ref<Circle>> query
) {
    for (auto [pos, circle] : query) {
        DrawCircle(
            pos.ptr->value.x,
            pos.ptr->value.y,
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
        .add_systems<spawn_system>(Schedule::STARTUP)
        .add_systems<
            movement_system,
            bounce_system
        >(Schedule::UPDATE)
        .add_systems<render_system>(Schedule::RENDER_2D)
        .run();

    return 0;
}
```

## Key Concepts

1.  **Component Composition**: Each ball is an `Entity` with `Position`, `Velocity`, and `Circle` components.
2.  **System Separation**: Movement, bouncing, and rendering logic are in separate, focused systems.
3.  **Query Iteration**: Each system efficiently iterates only over the entities that have the components it needs.
4.  **Resource Usage**: The `FrameTime` resource is used by the movement system to ensure frame-rate independent motion.

## Running the Example

Compile and run the example from your build directory.

## Next Steps

- Try [System Hierarchy](./system-hierarchy.md) for parent-child relationships.
- Check [Event Communication](./event-communication.md) for inter-system messaging.
