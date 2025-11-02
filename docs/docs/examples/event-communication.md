---
sidebar_position: 3
---

# Event Communication Example

Demonstrates inter-system communication using events.

## Overview

This example shows how systems can communicate without being tightly coupled.

- An input system sends a `FireEvent`.
- A bullet spawning system listens for `FireEvent` and creates bullets.
- A collision system detects hits and sends a `DamageEvent`.
- A health system listens for `DamageEvent` and sends a `DeathEvent` if health is depleted.
- A score system listens for `DeathEvent` to update the score.

## Event Definitions

```cpp
struct FireEvent {
    ecs::Entity shooter;
    Vec2f position;
    Vec2f direction;
};

struct DamageEvent {
    ecs::Entity target;
    int damage;
};

struct DeathEvent {
    ecs::Entity entity;
    int score_value;
};
```

## Systems

### Input System

Sends `FireEvent` when the player presses a key.

```cpp
void input_system(
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::With<Player>> query,
    ecs::Res<UserInput> input,
    ecs::EventWriter<FireEvent> events
) {
    if (input.ptr->isKeyJustPressed(KEY_SPACE)) {
        for (auto it = query.begin(); it != query.end(); ++it) {
            auto [pos, _] = *it;
            events.send(FireEvent{
                it.entity(),
                pos.ptr->value,
                Vec2f{1, 0}  // Fire to the right
            });
        }
    }
}
```

### Bullet Spawn System

Reacts to `FireEvent` by spawning bullet entities.

```cpp
void spawn_bullets(
    ecs::EventReader<FireEvent> events,
    ecs::Commands& commands
) {
    for (const auto& event : events) {
        commands.spawn(
            Position{event.position},
            Velocity{event.direction * 500.0f},
            Bullet{event.shooter}
        );
    }
}
```

### Collision System

Detects collisions and sends `DamageEvent`.

```cpp
void collision_system(
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::With<Bullet>> bullets,
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::With<Enemy>> enemies,
    ecs::EventWriter<DamageEvent> events,
    ecs::Commands& commands
) {
    for (auto it_bullet = bullets.begin(); it_bullet != bullets.end(); ++it_bullet) {
        auto [bullet_pos, _] = *it_bullet;
        for (auto it_enemy = enemies.begin(); it_enemy != enemies.end(); ++it_enemy) {
            auto [enemy_pos, __] = *it_enemy;
            if (distance(bullet_pos.ptr->value, enemy_pos.ptr->value) < 20.0f) {
                events.send(DamageEvent{it_enemy.entity(), 25});
                commands.despawn(it_bullet.entity());
            }
        }
    }
}
```

### Health System

Receives `DamageEvent` and sends `DeathEvent`.

```cpp
void health_system(
    ecs::EventReader<DamageEvent> damage_events,
    ecs::EventWriter<DeathEvent> death_events,
    ecs::Query<ecs::Mut<Health>> query,
    ecs::Commands& commands
) {
    for (const auto& event : damage_events) {
        if (auto* health = query.get_component_ptr<Health>(event.target)) {
            health->current -= event.damage;

            if (health->current <= 0) {
                death_events.send(DeathEvent{event.target, 100});
                commands.despawn(event.target);
            }
        }
    }
}
```

### Score System

Tracks score from `DeathEvent`.

```cpp
void score_system(
    ecs::EventReader<DeathEvent> events,
    ecs::ResMut<Score> score
) {
    for (const auto& event : events) {
        score.ptr->value += event.score_value;
        std::cout << "Score: " << score.ptr->value << "\n";
    }
}
```

## Application Setup

```cpp
int main() {
    Application{}
        // Register all event types
        .add_events<FireEvent, DamageEvent, DeathEvent>()

        // Add resources
        .insert_resource(UserInput{})
        .insert_resource(Score{0})

        // Add systems
        .add_systems<
            input_system,
            spawn_bullets,
            collision_system,
            health_system,
            score_system
        >(Schedule::UPDATE)
        .run();

    return 0;
}
```

## Key Concepts

1.  **Loose Coupling**: Systems communicate without needing to know about each other.
2.  **Event-Driven Logic**: Actions trigger events that other systems react to.
3.  **Multiple Receivers**: An event can be read by any number of systems.
4.  **Next-Frame Lifetime**: Events sent in one frame are readable in the next.

## Next Steps

- Learn more about [Events](../advanced/events.md)
- Check the [API Reference](../api/events.md)
