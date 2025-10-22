---
sidebar_position: 3
---

# Event Communication Example

Demonstrates inter-system communication using events.

## Overview

This example shows how systems communicate without tight coupling using events:
- Player input generates events
- Bullet spawn system reacts to events
- Collision system sends damage events
- Health system receives damage events

## Event Definitions

```cpp
struct FireEvent {
    Entity shooter;
    Vec2f position;
    Vec2f direction;
};

struct DamageEvent {
    Entity target;
    int damage;
};

struct DeathEvent {
    Entity entity;
    int score_value;
};
```

## Systems

### Input System

Sends fire events:

```cpp
void input_system(
    Query<Entity, Ref<Position>, With<Player>> query,
    Res<Input> input,
    EventWriter<FireEvent> events
) {
    if (input.ptr->fire_pressed) {
        for (auto [entity, pos, _] : query) {
            events.send(FireEvent{
                entity,
                pos.ptr->value,
                Vec2f{1, 0}  // Fire to the right
            });
        }
    }
}
```

### Bullet Spawn System

Reacts to fire events:

```cpp
void spawn_bullets(
    EventReader<FireEvent> events,
    Commands& commands
) {
    for (const auto& event : events.iter()) {
        commands.spawn(
            Position{event.position},
            Velocity{event.direction * 500.0f},
            Bullet{event.shooter}
        );
    }
}
```

### Collision System

Detects collisions and sends damage events:

```cpp
void collision_system(
    Query<Entity, Ref<Position>, With<Bullet>> bullets,
    Query<Entity, Ref<Position>, With<Enemy>> enemies,
    EventWriter<DamageEvent> events,
    Commands& commands
) {
    for (auto [bullet_entity, bullet_pos, _] : bullets) {
        for (auto [enemy_entity, enemy_pos, _] : enemies) {
            if (distance(bullet_pos.ptr->value, enemy_pos.ptr->value) < 20.0f) {
                // Send damage event
                events.send(DamageEvent{enemy_entity, 25});
                
                // Despawn bullet
                commands.entity(bullet_entity).despawn();
            }
        }
    }
}
```

### Health System

Receives damage events:

```cpp
void health_system(
    EventReader<DamageEvent> damage_events,
    EventWriter<DeathEvent> death_events,
    Query<Entity, Mut<Health>> query,
    Commands& commands
) {
    for (const auto& event : damage_events.iter()) {
        if (auto health = query.get<Mut<Health>>(event.target)) {
            health.ptr->current -= event.damage;
            
            // Check for death
            if (health.ptr->current <= 0) {
                death_events.send(DeathEvent{event.target, 100});
                commands.entity(event.target).despawn();
            }
        }
    }
}
```

### Score System

Tracks score from death events:

```cpp
void score_system(
    EventReader<DeathEvent> events,
    ResMut<Score> score
) {
    for (const auto& event : events.iter()) {
        score.ptr->value += event.score_value;
        std::cout << "Score: " << score.ptr->value << "\n";
    }
}
```

## Application Setup

```cpp
int main() {
    Application{}
        // Register events
        .add_event<FireEvent>()
        .add_event<DamageEvent>()
        .add_event<DeathEvent>()
        
        // Resources
        .insert_resource(Input{})
        .insert_resource(Score{0})
        
        // Systems
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

## Event Flow

```
Player Input
    ↓ (FireEvent)
Spawn Bullets
    ↓ (bullets move)
Collision Detection
    ↓ (DamageEvent)
Health System
    ↓ (DeathEvent)
Score System
```

## Key Concepts

1. **Loose Coupling**: Systems don't directly call each other
2. **Event-Driven**: Actions trigger events that others can react to
3. **Multiple Receivers**: Multiple systems can read the same event
4. **One Frame Lifetime**: Events are cleared each frame

## Running

```bash
./r-engine__event_writer_reader
```

## Next Steps

- Learn more about [Events](../advanced/events.md)
- Check [API Reference](../api/events.md)
