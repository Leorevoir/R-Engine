---
sidebar_position: 2
---

# Architecture

Understanding the R-Engine ECS architecture will help you write more efficient and maintainable code.

## System Overview

```
┌─────────────────────────────────────────────────────────┐
│                      Application                         │
│  ┌────────────────────────────────────────────────────┐ │
│  │                    Scheduler                        │ │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐         │ │
│  │  │ STARTUP  │  │  UPDATE  │  │  RENDER  │  ...    │ │
│  │  └──────────┘  └──────────┘  └──────────┘         │ │
│  └────────────────────────────────────────────────────┘ │
│                                                          │
│  ┌────────────────────────────────────────────────────┐ │
│  │                     Scene                           │ │
│  │  ┌──────────────┐  ┌──────────────┐               │ │
│  │  │   Storage    │  │  Resources   │               │ │
│  │  │  (Entities)  │  │   (Global)   │               │ │
│  │  └──────────────┘  └──────────────┘               │ │
│  │  ┌──────────────┐  ┌──────────────┐               │ │
│  │  │    Events    │  │   Commands   │               │ │
│  │  └──────────────┘  └──────────────┘               │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

## Execution Flow

### 1. Startup Phase

```cpp
Application{}
    .add_systems<setup_system, init_resources>(Schedule::STARTUP)
    .run();
```

Systems in the STARTUP schedule run once before the main loop:

```
┌─────────────┐
│   START     │
└──────┬──────┘
       │
       v
┌─────────────────┐
│ STARTUP Systems │
└──────┬──────────┘
       │
       v
┌─────────────┐
│  Main Loop  │
└─────────────┘
```

### 2. Update Loop

The main loop executes repeatedly:

```
┌──────────────────────────────────────────┐
│            UPDATE Schedule                │
│                                           │
│  Input → Logic → Physics → Rendering     │
│    ↓       ↓        ↓          ↓         │
│  Systems running with dependencies       │
│                                           │
│  Commands applied between systems        │
│  Events dispatched between systems       │
└──────────────────────────────────────────┘
       │
       v (Next frame)
```

### 3. System Dependencies

Systems can specify execution order:

```cpp
Application{}
    .add_systems<input_system, movement_system, render_system>(Schedule::UPDATE)
    .after<input_system>()  // movement_system runs after input_system
    .run();
```

## Archetype-Based Storage

R-Engine uses **archetype-based storage** for optimal performance.

### What is an Archetype?

An archetype is a unique combination of component types. Entities with the same components share the same archetype:

```
Archetype A: [Position, Velocity]
  Entity 1: Position{0,0}, Velocity{1,0}
  Entity 2: Position{5,3}, Velocity{-1,2}
  ...

Archetype B: [Position, Velocity, Health]
  Entity 10: Position{2,1}, Velocity{0,1}, Health{100}
  Entity 11: Position{-3,4}, Velocity{2,0}, Health{75}
  ...
```

### Benefits

- **Cache-friendly**: Components of the same type are stored contiguously
- **Fast iteration**: Queries iterate over entire archetypes at once
- **Memory efficient**: No pointer indirection

### Archetype Changes

When components are added or removed, entities move between archetypes:

```cpp
// Entity starts in archetype [Position, Velocity]
commands.spawn(Position{}, Velocity{});

// Adding Health moves entity to archetype [Position, Velocity, Health]
entity_commands.insert(Health{100});

// Removing Velocity moves entity to archetype [Position, Health]
entity_commands.remove<Velocity>();
```

## Memory Layout

```
┌──────────────────────────────────────────────────┐
│              Archetype Storage                    │
├──────────────────────────────────────────────────┤
│ Archetype [Position, Velocity]                   │
│ ┌──────────────────────────────────────────────┐ │
│ │ Positions: [P1][P2][P3][P4]...               │ │
│ │ Velocities: [V1][V2][V3][V4]...              │ │
│ │ Entity IDs: [E1][E2][E3][E4]...              │ │
│ └──────────────────────────────────────────────┘ │
│                                                   │
│ Archetype [Position, Health]                     │
│ ┌──────────────────────────────────────────────┐ │
│ │ Positions: [P5][P6][P7]...                   │ │
│ │ Health: [H5][H6][H7]...                      │ │
│ │ Entity IDs: [E5][E6][E7]...                  │ │
│ └──────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────┘
```

## Query Execution

Queries iterate efficiently over matching archetypes:

```cpp
void system(Query<Mut<Position>, Ref<Velocity>> query) {
    // Automatically iterates over all archetypes
    // that have Position AND Velocity
    for (auto [pos, vel] : query) {
        // Process...
    }
}
```

### Query Matching

```
Available Archetypes:
  [Position, Velocity]       ← Matches ✓
  [Position, Velocity, HP]   ← Matches ✓
  [Position, Sprite]         ← No match ✗
  [Velocity]                 ← No match ✗
```

## Command Buffer

Commands are deferred to avoid iterator invalidation:

```
System Execution:
┌────────────────────┐
│   System runs      │
│   Queries iterate  │
│   Commands queued  │
└─────────┬──────────┘
          │
          v
┌────────────────────┐
│ Commands applied   │
│ Entities spawned   │
│ Components added   │
│ Archetypes updated │
└────────────────────┘
```

This ensures that modifications don't affect the current iteration.

## Next Steps

- Explore [Core Concepts](./core-concepts/index.md) for detailed explanations
- See [Storage System](./storage.md) for implementation details
- Check [Examples](./examples/index.md) for practical code
