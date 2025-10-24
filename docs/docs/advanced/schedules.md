---
sidebar_position: 4
---

# Schedules

Schedules organize system execution into distinct phases.

## Built-in Schedules

R-Engine provides several default schedules:

```cpp
Schedule::STARTUP    // Run once at application start
Schedule::UPDATE     // Main game loop (every frame)
Schedule::RENDER_2D  // 2D rendering
Schedule::RENDER_3D  // 3D rendering
// ... and more
```

## Using Schedules

```cpp
Application{}
    .add_systems<setup>(Schedule::STARTUP)
    .add_systems<input, movement>(Schedule::UPDATE)
    .add_systems<render>(Schedule::RENDER_2D)
    .run();
```

## System Ordering

### Within a Schedule

Use `.after<System>()` and `.before<System>()` to define explicit dependencies.

```cpp
// Guarantees movement_system runs after input_system in the same frame
app.add_systems<movement_system>(Schedule::UPDATE)
   .after<input_system>();
```

### Between Schedules

Schedules run in a predefined order each frame:

```
(STARTUP once)
↓
PRE_UPDATE
↓
UPDATE
↓
POST_UPDATE
↓
RENDER_3D
↓
RENDER_2D
↓
(repeat from PRE_UPDATE)
```

## System Sets

Group related systems to manage their dependencies collectively. A set is defined using a simple, empty `struct`.

```cpp
// Define sets as simple types
struct InputSet {};
struct LogicSet {};
struct PhysicsSet {};

// Add systems to sets
app.add_systems<keyboard_input, mouse_input>(Schedule::UPDATE)
   .in_set<InputSet>();

// Order entire sets relative to each other
app.add_systems<player_movement, enemy_ai>(Schedule::UPDATE)
   .in_set<LogicSet>()
   .after<InputSet>();

app.add_systems<collision_detection>(Schedule::UPDATE)
   .in_set<PhysicsSet>()
   .after<LogicSet>();
```

## Best Practices

### Organize by Phase

```cpp
// Input phase
app.add_systems<keyboard_input, mouse_input>(Schedule::PRE_UPDATE);

// Logic phase
app.add_systems<player_movement, enemy_ai>(Schedule::UPDATE);

// Physics phase
app.add_systems<collision, physics_solver>(Schedule::POST_UPDATE);

// Render phase
app.add_systems<sprite_render, ui_render>(Schedule::RENDER_2D);
```

### Use Sets for Dependencies

```cpp
struct LogicSet {};
struct RenderingSet {};

app.add_systems<game_logic_a, game_logic_b>(Schedule::UPDATE)
   .in_set<LogicSet>();

app.add_systems<render_a, render_b>(Schedule::RENDER_2D)
   .in_set<RenderingSet>();

// To ensure rendering happens after logic, you can configure the sets
app.configure_sets<RenderingSet>(Schedule::UPDATE)
   .after<LogicSet>();
```

## Example: Complete Setup

```cpp
Application{}
    // Startup
    .add_systems<load_assets, spawn_player>(Schedule::STARTUP)

    // Pre-update: Input
    .add_systems<input_system>(Schedule::PRE_UPDATE)

    // Update: Game Logic
    .add_systems<
        player_movement,
        enemy_ai,
        projectile_movement
    >(Schedule::UPDATE)
    .after<input_system>()

    // Post-update: Physics
    .add_systems<collision_detection, physics_solver>(Schedule::POST_UPDATE)

    // Render
    .add_systems<render_sprites, render_ui>(Schedule::RENDER_2D)
    .run();
```

## Next Steps

- See [Examples](../examples/index.md) for complete applications
- Check [API Reference](../api/index.md) for technical details
