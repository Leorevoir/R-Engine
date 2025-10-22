---
sidebar_position: 5
---

# Events API

Event communication between systems.

## EventWriter\<T\>

Send events to other systems.

### send()

```cpp
void send(T event)
```

Send an event to be sent.

**Example**:
```cpp
void system(EventWriter<CollisionEvent> events) {
    events.send(CollisionEvent{entity_a, entity_b});
}
```

## EventReader\<T\>

Receive events from other systems.

### iter()

```cpp
Iterator iter()
```

Iterate over all events sent this frame.

**Example**:
```cpp
void system(EventReader<CollisionEvent> events) {
    for (const auto& event : events.iter()) {
        // Handle event
        std::cout << "Collision!\n";
    }
}
```

## Event Lifecycle

Events last for one frame:

```
Frame N:
  - EventWriter sends event
  - EventReader(s) read event
  - Event cleared at frame end

Frame N+1:
  - Event no longer available
```

## Usage Pattern

```cpp
// 1. Define event
struct DamageEvent {
    Entity target;
    int amount;
};

// 2. Register event
Application{}
    .add_event<DamageEvent>()
    // ...

// 3. Send events
void combat_system(EventWriter<DamageEvent> events) {
    events.send(DamageEvent{enemy, 25});
}

// 4. Read events
void health_system(
    EventReader<DamageEvent> events,
    Query<Mut<Health>> query
) {
    for (const auto& event : events.iter()) {
        if (auto health = query.get<Mut<Health>>(event.target)) {
            health.ptr->current -= event.amount;
        }
    }
}
```

## Multiple Readers

Multiple systems can read the same event:

```cpp
// System 1
void damage_system(EventReader<CollisionEvent> events) {
    for (const auto& e : events.iter()) {
        // Apply damage
    }
}

// System 2
void sound_system(EventReader<CollisionEvent> events) {
    for (const auto& e : events.iter()) {
        // Play sound
    }
}
```

## See Also

- [Events Guide](../advanced/events.md)
- [Event Communication Example](../examples/event-communication.md)
