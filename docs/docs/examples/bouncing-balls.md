---
sidebar_position: 1
---

# Bouncing Balls Example

A simple physics simulation demonstrating entities, components, and systems.

## Overview

This example creates multiple ball entities that bounce around the screen, demonstrating:
- Entity spawning
- Component-based data
- System logic
- Query iteration

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

Creates ball entities:

```cpp
void spawn_system(Commands& commands) {
    for (int i = 0; i < 10; i++) {
        commands.spawn(
            Position{Vec2f{rand() % 800, rand() % 600}},
            Velocity{Vec2f{rand_float(-5, 5), rand_float(-5, 5)}},
            Circle{20.0f, random_color()}
        );
    }
}
```

### Movement System

Updates positions based on velocity:

```cpp
void movement_system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<DeltaTime> time
) {
    for (auto [pos, vel] : query) {
        pos.ptr->value += vel.ptr->value * time.ptr->dt;
    }
}
```

### Bounce System

Bounces balls off screen edges:

```cpp
void bounce_system(
    Query<Ref<Position>, Mut<Velocity>, Ref<Circle>> query
) {
    for (auto [pos, vel, circle] : query) {
        // Left/right walls
        if (pos.ptr->value.x - circle.ptr->radius < 0 ||
            pos.ptr->value.x + circle.ptr->radius > 800) {
            vel.ptr->value.x *= -1;
        }
        
        // Top/bottom walls
        if (pos.ptr->value.y - circle.ptr->radius < 0 ||
            pos.ptr->value.y + circle.ptr->radius > 600) {
            vel.ptr->value.y *= -1;
        }
    }
}
```

### Render System

Draws the balls:

```cpp
void render_system(
    Query<Ref<Position>, Ref<Circle>> query
) {
    for (auto [pos, circle] : query) {
        draw_circle(
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
        .insert_resource(DeltaTime{})
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

1. **Component Composition**: Each ball is an entity with Position, Velocity, and Circle components
2. **System Separation**: Movement, bouncing, and rendering are separate systems
3. **Query Iteration**: Each system efficiently iterates over relevant entities
4. **Resource Usage**: DeltaTime resource provides frame timing

## Running the Example

```bash
cd R-Engine/build
./r-engine__bouncing_balls
```

## Next Steps

- Try [System Hierarchy](./system-hierarchy.md) for parent-child relationships
- Check [Event Communication](./event-communication.md) for inter-system messaging
