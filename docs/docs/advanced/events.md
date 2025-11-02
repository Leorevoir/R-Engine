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
    ecs::Entity entity_a;
    ecs::Entity entity_b;
    Vec2f impact_point;
};
```

## Registering Events

Add events to your application using the `add_events` method:

```cpp
Application{}
    .add_events<CollisionEvent, DamageEvent>()
    .run();
```

## Sending Events

Use `ecs::EventWriter<T>` to send events:

```cpp
void collision_system(
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::Ref<Collider>> query,
    ecs::EventWriter<CollisionEvent> events
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

Use `ecs::EventReader<T>` to receive events. The reader is directly iterable.

```cpp
void damage_system(
    ecs::EventReader<CollisionEvent> collision_events,
    ecs::Query<ecs::Mut<Health>> query
) {
    for (const auto& event : collision_events) {
        // Handle collision event
        if (auto health = query.get_component_ptr<Health>(event.entity_a)) {
            health->value -= 10;
        }
    }
}
```

## Event Lifecycle

Events are managed with a **double-buffer** system. This means events sent in a given frame are only available to be read in the **next frame**.

```
Frame N:
System A sends event E1
System B sends event E2
...
(Events E1 and E2 are stored in the "write" buffer for this frame)
...
EVENT_CLEANUP schedule runs
Event buffers are swapped. The "write" buffer becomes the "read" buffer.

Frame N+1:
System C reads events E1 and E2 from the "read" buffer.
...
EVENT_CLEANUP schedule runs
The "read" buffer (containing E1, E2) is cleared, and buffers are swapped again.
```

## Common Event Patterns

### Input Events

```cpp
struct KeyPressEvent {
    int key_code;
    bool pressed;
};

void input_system(ecs::EventWriter<KeyPressEvent> events) {
    // Poll input
    if (IsKeyPressed(KEY_SPACE)) {
        events.send(KeyPressEvent{KEY_SPACE, true});
    }
}

void player_system(ecs::EventReader<KeyPressEvent> events) {
    for (const auto& event : events) {
        if (event.key_code == KEY_SPACE && event.pressed) {
            // Jump!
        }
    }
}
```

### Game Events

```cpp
struct EnemyDefeatedEvent {
    ecs::Entity enemy;
    int score_value;
};

void combat_system(
    ecs::Query<ecs::Entity, ecs::Mut<Health>, ecs::With<Enemy>> query,
    ecs::EventWriter<EnemyDefeatedEvent> events,
    ecs::Commands& commands
) {
    for (auto [entity, health, _] : query) {
        if (health.ptr->current <= 0) {
            events.send(EnemyDefeatedEvent{entity, 100});
            commands.despawn(entity);
        }
    }
}

void score_system(
    ecs::EventReader<EnemyDefeatedEvent> events,
    ecs::ResMut<Score> score
) {
    for (const auto& event : events) {
        score.ptr->value += event.score_value;
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
- Don't expect events to be readable in the same frame they are sent.

## Example: Complete Event Flow

```cpp
// Event definitions
struct BulletFiredEvent {
    ecs::Entity shooter;
    Vec2f position;
    Vec2f direction;
};

struct HitEvent {
    ecs::Entity target;
    int damage;
};

// System 1: Fire bullets
void shooting_system(
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::With<Player>> query,
    ecs::Res<UserInput> input,
    ecs::EventWriter<BulletFiredEvent> events
) {
    if (input.ptr->isKeyJustPressed(KEY_SPACE)) {
        for (auto it = query.begin(); it != query.end(); ++it) {
            auto [pos, _] = *it;
            events.send(BulletFiredEvent{
                it.entity(), pos.ptr->value, Vec2f{1, 0}
            });
        }
    }
}

// System 2: Spawn bullets from events
void bullet_spawn_system(
    ecs::EventReader<BulletFiredEvent> events,
    ecs::Commands& commands
) {
    for (const auto& event : events) {
        commands.spawn(
            Bullet{},
            Position{event.position},
            Velocity{event.direction * 10.0f}
        );
    }
}

// System 3: Detect hits
void collision_system(
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::With<Bullet>> bullets,
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::With<Enemy>> enemies,
    ecs::EventWriter<HitEvent> events
) {
    for (auto it_bullet = bullets.begin(); it_bullet != bullets.end(); ++it_bullet) {
        auto [bullet_pos, _] = *it_bullet;
        for (auto it_enemy = enemies.begin(); it_enemy != enemies.end(); ++it_enemy) {
            auto [enemy_pos, _] = *it_enemy;
            if (distance(bullet_pos.ptr->value, enemy_pos.ptr->value) < 10.0f) {
                events.send(HitEvent{it_enemy.entity(), 25});
            }
        }
    }
}

// System 4: Apply damage
void damage_system(
    ecs::EventReader<HitEvent> events,
    ecs::Query<ecs::Mut<Health>> query
) {
    for (const auto& event : events) {
        if (auto* health = query.get_component_ptr<Health>(event.target)) {
            health->current -= event.damage;
        }
    }
}

// Application
Application{}
    .add_events<BulletFiredEvent, HitEvent>()
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
