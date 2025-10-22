---
sidebar_position: 1
---

# Events

Events provide asynchronous, decoupled communication between systems.

## What are Events?

Events are messages sent by one system and received by others, enabling loose coupling between game systems.

```cpp
// Define an event
struct CollisionEvent {
    Entity entity_a;
    Entity entity_b;
    Vec2 impact_point;
};
```

## Registering Events

Add events to your application:

```cpp
Application{}
    .add_event<CollisionEvent>()
    .add_event<DamageEvent>()
    .run();
```

## Sending Events

Use `EventWriter<T>` to send events:

```cpp
void collision_system(
    Query<Entity, Ref<Position>, Ref<Collider>> query,
    EventWriter<CollisionEvent> events
) {
    // Check collisions...
    if (collision_detected) {
        events.send(CollisionEvent{
            .entity_a = e1,
            .entity_b = e2,
            .impact_point = point
        });
    }
}
```

## Reading Events

Use `EventReader<T>` to receive events:

```cpp
void damage_system(
    EventReader<CollisionEvent> events,
    Query<Mut<Health>> query
) {
    for (const auto& event : events.iter()) {
        // Handle collision event
        auto health = query.get<Mut<Health>>(event.entity_a);
        if (health) {
            health->value -= 10;
        }
    }
}
```

## Event Lifecycle

Events are cleared after all readers have processed them:

```
Frame N:
  System A sends event
  System B reads event
  System C reads event
  Event cleared at end of frame

Frame N+1:
  Event no longer available
```

## Common Event Patterns

### Input Events

```cpp
struct KeyPressEvent {
    int key_code;
    bool pressed;
};

void input_system(EventWriter<KeyPressEvent> events) {
    // Poll input
    if (key_pressed(KEY_SPACE)) {
        events.send(KeyPressEvent{KEY_SPACE, true});
    }
}

void player_system(EventReader<KeyPressEvent> events) {
    for (const auto& event : events.iter()) {
        if (event.key_code == KEY_SPACE && event.pressed) {
            // Jump!
        }
    }
}
```

### Game Events

```cpp
struct EnemyDefeatedEvent {
    Entity enemy;
    int score_value;
};

void combat_system(
    Query<Entity, Mut<Health>, With<Enemy>> query,
    EventWriter<EnemyDefeatedEvent> events,
    Commands& commands
) {
    for (auto [entity, health, _] : query) {
        if (health->current <= 0) {
            events.send(EnemyDefeatedEvent{entity, 100});
            commands.entity(entity).despawn();
        }
    }
}

void score_system(
    EventReader<EnemyDefeatedEvent> events,
    ResMut<Score> score
) {
    for (const auto& event : events.iter()) {
        score->value += event.score_value;
    }
}
```

## Best Practices

### ✅ Do

- Use events for loose coupling between systems
- Keep event data lightweight
- Use descriptive event names

### ❌ Don't

- Don't use events for every system interaction (queries are fine for direct access)
- Don't store large data in events
- Don't expect events to persist across frames

## Example: Complete Event Flow

```cpp
// Event definitions
struct BulletFiredEvent {
    Entity shooter;
    Vec2 position;
    Vec2 direction;
};

struct HitEvent {
    Entity target;
    int damage;
};

// System 1: Fire bullets
void shooting_system(
    Query<Entity, Ref<Position>, With<Player>> query,
    Res<Input> input,
    EventWriter<BulletFiredEvent> events
) {
    if (input->fire_pressed) {
        for (auto [entity, pos, _] : query) {
            events.send(BulletFiredEvent{
                entity, pos->value, Vec2{1, 0}
            });
        }
    }
}

// System 2: Spawn bullets from events
void bullet_spawn_system(
    EventReader<BulletFiredEvent> events,
    Commands& commands
) {
    for (const auto& event : events.iter()) {
        commands.spawn(
            Bullet{},
            Position{event.position},
            Velocity{event.direction * 10.0f}
        );
    }
}

// System 3: Detect hits
void collision_system(
    Query<Entity, Ref<Position>, With<Bullet>> bullets,
    Query<Entity, Ref<Position>, With<Enemy>> enemies,
    EventWriter<HitEvent> events
) {
    for (auto [bullet_e, bullet_pos, _] : bullets) {
        for (auto [enemy_e, enemy_pos, _] : enemies) {
            if (distance(bullet_pos->value, enemy_pos->value) < 10.0f) {
                events.send(HitEvent{enemy_e, 25});
            }
        }
    }
}

// System 4: Apply damage
void damage_system(
    EventReader<HitEvent> events,
    Query<Mut<Health>> query
) {
    for (const auto& event : events.iter()) {
        if (auto health = query.get<Mut<Health>>(event.target)) {
            health->current -= event.damage;
        }
    }
}

// Application
Application{}
    .add_event<BulletFiredEvent>()
    .add_event<HitEvent>()
    .add_systems<
        shooting_system,
        bullet_spawn_system,
        collision_system,
        damage_system
    >(Schedule::UPDATE)
    .run();
```

## Next Steps

- Learn about [Hierarchies](./hierarchies.md)
- Check [Examples](../examples/event-communication.md)
