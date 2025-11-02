---
sidebar_position: 5
---

# Events API

Event communication between systems.

## EventWriter\<T\>

A system parameter for sending events to other systems.

### send()

```cpp
void send(const EventT& event);
void send(EventT&& event);
```

Sends an event of type `T`.

**Example**:

```cpp
void system(ecs::EventWriter<CollisionEvent> events) {
    events.send(CollisionEvent{entity_a, entity_b});
}
```

## EventReader\<T\>

A system parameter for receiving events from other systems.

### Iteration

`EventReader` is directly iterable.

```cpp
void system(ecs::EventReader<CollisionEvent> events) {
    for (const auto& event : events) {
        // Handle event
        std::cout << "Collision!\n";
    }
}
```

### has_events()

```cpp
bool has_events() const noexcept;
```

Checks if there are any events to be read. Useful for run conditions.

## Event Lifecycle

Events are managed with a **double-buffer** system. Events sent in frame `N` are available to be read by any system in **frame `N+1`**. At the end of frame `N+1`, during the `EVENT_CLEANUP` schedule, the buffer that was read from is cleared.

```
Frame N:
  - EventWriter sends event E1.
  - Event E1 is stored in the "write" buffer.
  - During EVENT_CLEANUP, buffers are swapped.

Frame N+1:
  - EventReader(s) can now read event E1 from the "read" buffer.
  - EventWriter sends new events to the "write" buffer.
  - During EVENT_CLEANUP, the "read" buffer (with E1) is cleared, and buffers are swapped again.
```

## Usage Pattern

```cpp
// 1. Define event
struct DamageEvent {
    ecs::Entity target;
    int amount;
};

// 2. Register event in the application
Application{}
    .add_events<DamageEvent>()
    // ...

// 3. Send events from a system
void combat_system(ecs::EventWriter<DamageEvent> events) {
    events.send(DamageEvent{enemy, 25});
}

// 4. Read events in another system
void health_system(
    ecs::EventReader<DamageEvent> events,
    ecs::Query<ecs::Mut<Health>> query
) {
    for (const auto& event : events) {
        if (auto* health = query.get_component_ptr<Health>(event.target)) {
            health->current -= event.amount;
        }
    }
}
```

## Multiple Readers

Multiple systems can have an `EventReader` for the same event type. All of them will receive all events sent that frame.

```cpp
// System 1
void damage_system(ecs::EventReader<CollisionEvent> events) {
    for (const auto& e : events) {
        // Apply damage
    }
}

// System 2
void sound_system(ecs::EventReader<CollisionEvent> events) {
    for (const auto& e : events) {
        // Play sound
    }
}
```

## See Also

- [Events Guide](../advanced/events.md)
- [Event Communication Example](../examples/event-communication.md)
