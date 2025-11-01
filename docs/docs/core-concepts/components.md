---
sidebar_position: 2
---

# Components

Components are pure data structures that define what an entity **is** or **has**. They contain no logic, only data.

## What is a Component?

A component is a simple struct or class that holds data. Components are attached to entities to give them properties.

```cpp
// Simple position component
struct Position {
    float x;
    float y;
};

// Health component
struct Health {
    int current;
    int max;
};

// Marker component (no data)
struct Player {};
```

## Creating Components

### POD Components (Recommended)

Use Plain Old Data (POD) structs for best performance:

```cpp
struct Velocity {
    float x = 0.0f;
    float y = 0.0f;
};

struct Color {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
};
```

### Complex Components

Components can contain methods for convenience:

```cpp
struct Health {
    float current = 100.0f;
    float max = 100.0f;
    
    bool is_alive() const {
        return current > 0.0f;
    }
    
    float percentage() const {
        return current / max;
    }
    
    void damage(float amount) {
        current = std::max(0.0f, current - amount);
    }
};
```

:::tip Best Practice
Keep logic minimal in components. Most logic should be in systems.
:::

## Component Types

### Data Components

Store state or properties:

```cpp
struct Transform {
    Vec3 position;
    Quaternion rotation;
    Vec3 scale = {1, 1, 1};
};

struct Sprite {
    Texture texture;
    Rect source_rect;
};
```

### Marker Components

Tag entities without storing data:

```cpp
struct Player {};
struct Enemy {};
struct Bullet {};
struct Dead {};

// Usage in query
void player_system(Query<Ref<Position>, With<Player>> query) {
    // Only processes entities with Player component
}
```

### Flag Components

Store boolean states efficiently:

```cpp
struct Flags {
    bool is_visible : 1;
    bool is_solid : 1;
    bool is_animated : 1;
    bool is_hostile : 1;
};
```

## Adding Components

### At Spawn Time

```cpp
void spawn_system(Commands& commands) {
    // Spawn entity with multiple components
    commands.spawn(
        Position{100.0f, 200.0f},
        Velocity{0.0f, 0.0f},
        Health{100},
        Player{}
    );
}
```

### After Creation

```cpp
void system(Query<Entity, Without<Health>> query, Commands& commands) {
    for (auto [entity, _] : query) {
        // Add component to existing entity
        commands.entity(entity).insert(Health{50});
    }
}
```

## Removing Components

```cpp
void system(Query<Entity, With<Dead>> query, Commands& commands) {
    for (auto [entity, _] : query) {
        // Remove specific component
        commands.entity(entity).remove<AI>();
        
        // Or despawn entire entity
        commands.entity(entity).despawn();
    }
}
```

## Accessing Components

### Mutable Access

Use `Mut<T>` to modify components:

```cpp
void movement_system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<DeltaTime> time
) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x * time->dt;
        pos->y += vel->y * time->dt;
    }
}
```

### Read-Only Access

Use `Ref<T>` for read-only access:

```cpp
void render_system(Query<Ref<Position>, Ref<Sprite>> query) {
    for (auto [pos, sprite] : query) {
        draw(sprite->texture, pos->x, pos->y);
    }
}
```

## Component Design Patterns

### Composition Over Inheritance

Instead of inheritance hierarchies, compose entities from components:

```cpp
// ❌ Bad: Deep inheritance
class GameObject {};
class Character : public GameObject {};
class Player : public Character {};
class Mage : public Player {};

// ✅ Good: Composition
struct Player {};
struct Character {};
struct MagicUser {};

// Create mage by combining components
commands.spawn(Player{}, Character{}, MagicUser{});
```

### Single Responsibility

Keep components focused on one concern:

```cpp
// ❌ Bad: Kitchen sink component
struct GameObject {
    Position position;
    Velocity velocity;
    Health health;
    Sprite sprite;
    // Too many responsibilities!
};

// ✅ Good: Focused components
struct Position { Vec2 value; };
struct Velocity { Vec2 value; };
struct Health { int value; };
struct Sprite { Texture tex; };
```

### Data-Oriented Design

Store data in arrays (handled by ECS):

```cpp
// ECS stores components in contiguous arrays:
// Positions: [P1][P2][P3][P4]...
// Velocities: [V1][V2][V3][V4]...
// Very cache-friendly!
```

## Best Practices

### ✅ Do

- Keep components small and focused
- Use POD structs when possible
- Use marker components for categorization
- Prefer multiple small components over one large component

```cpp
// Good: Focused components
struct Position { Vec2 value; };
struct Velocity { Vec2 value; };
struct CircleCollider { float radius; };
```

### ❌ Don't

- Don't add logic to components (use systems)
- Don't create god components
- Don't use inheritance between components

```cpp
// Bad: Too much logic in component
struct Enemy {
    void update() { /* logic here */ }
    void draw() { /* rendering here */ }
    // Should be in systems!
};
```

## Example: Full Entity Setup

```cpp
// Define components
struct Position { float x, y; };
struct Velocity { float x, y; };
struct Circle { float radius; Color color; };
struct Player {};

// Spawn player entity
void setup(Commands& commands) {
    commands.spawn(
        Position{400.0f, 300.0f},
        Velocity{0.0f, 0.0f},
        Circle{25.0f, Color::Blue},
        Player{}
    );
}

// Systems operate on components
void movement(Query<Mut<Position>, Ref<Velocity>> query) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x;
        pos->y += vel->y;
    }
}

void render(Query<Ref<Position>, Ref<Circle>> query) {
    for (auto [pos, circle] : query) {
        draw_circle(pos->x, pos->y, circle->radius, circle->color);
    }
}
```

## Next Steps

- Learn about [Systems](./systems.md) to add logic
- Explore [Queries](./queries.md) to access components efficiently
- See [Commands](./commands.md) for adding/removing components
