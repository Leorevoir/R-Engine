---
sidebar_position: 4
---

# Advanced Features

This section covers advanced ECS features that give you more control and flexibility.

## Overview

Once you understand the core concepts, these advanced features will help you build more sophisticated applications:

### Events

Asynchronous communication between systems using an event-driven architecture.

[Learn about Events →](./events.md)

### Hierarchies

Parent-child relationships between entities for scene graphs and nested structures.

[Learn about Hierarchies →](./hierarchies.md)

### Run Conditions

Control when systems execute based on runtime conditions.

[Learn about Run Conditions →](./run-conditions.md)

### Schedules

Organize and order system execution across multiple stages.

[Learn about Schedules →](./schedules.md)

## When to Use Advanced Features

- **Events**: When systems need to communicate without tight coupling
- **Hierarchies**: For transforms, UI layouts, or nested game objects
- **Run Conditions**: To enable/disable systems dynamically
- **Schedules**: To organize complex system dependencies

## Example: Combining Features

```cpp
// Event definition
struct GameStartEvent {};

// System with run condition
bool game_is_running(Res<GameState> state) {
    return state->running;
}

// Application setup
Application{}
    // Events
    .add_event<GameStartEvent>()
    
    // Systems with conditions
    .add_systems<game_logic>(Schedule::UPDATE)
    .run_if(run_condition<game_is_running>())
    
    // Hierarchical setup
    .add_systems<setup_ui_hierarchy>(Schedule::STARTUP)
    .run();
```

## Next Steps

Start with [Events](./events.md) to learn about inter-system communication.
