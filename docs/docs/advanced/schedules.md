---
sidebar_position: 4
---

# Schedules

Schedules organize system execution into distinct phases.

## Built-in Schedules

R-Engine provides several default schedules defined in the `Schedule` enum:

- `PRE_STARTUP`: Runs once before the main startup phase. Useful for setting up foundational resources.
- `STARTUP`: Runs once when the application starts, before the main loop. Ideal for spawning initial entities and loading assets.
- `UPDATE`: The main game loop schedule, runs every frame. Most game logic goes here.
- `FIXED_UPDATE`: Runs on a fixed time step, suitable for physics calculations.
- `BEFORE_RENDER_3D` / `RENDER_3D` / `AFTER_RENDER_3D`: Phases for 3D rendering.
- `BEFORE_RENDER_2D` / `RENDER_2D` / `AFTER_RENDER_2D`: Phases for 2D rendering and UI.
- `EVENT_CLEANUP`: Runs at the end of the frame to clear events.
- `SHUTDOWN`: Runs once when the application is closing.

## Using Schedules

You assign systems to a schedule when adding them to the application.

```cpp
Application{}
    .add_systems<setup_scene>(Schedule::STARTUP)
    .add_systems<player_input, enemy_ai>(Schedule::UPDATE)
    .add_systems<physics_step>(Schedule::FIXED_UPDATE)
    .add_systems<render_models>(Schedule::RENDER_3D)
    .add_systems<render_ui>(Schedule::RENDER_2D)
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

### Frame Execution Order

Schedules run in a predefined order each frame:

```
(PRE_STARTUP -> STARTUP once)
↓
<main loop>
  UPDATE
  ↓
  FIXED_UPDATE (zero or more times)
  ↓
  BEFORE_RENDER_3D -> RENDER_3D -> AFTER_RENDER_3D
  ↓
  BEFORE_RENDER_2D -> RENDER_2D -> AFTER_RENDER_2D
  ↓
  EVENT_CLEANUP
(repeat main loop)
↓
SHUTDOWN
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

app.add_systems<player_movement, enemy_ai>(Schedule::UPDATE)
   .in_set<LogicSet>();

// Order logic to run after input
app.configure_sets<LogicSet>(Schedule::UPDATE)
   .after<InputSet>();

// You can also specify dependencies directly when adding systems
app.add_systems<collision_detection>(Schedule::UPDATE)
   .in_set<PhysicsSet>()
   .after<LogicSet>(); // The PhysicsSet runs after the LogicSet
```

## Best Practices

### Organize by Phase

Use the appropriate schedule for each type of logic.

```cpp
// Input phase
app.add_systems<keyboard_input, mouse_input>(Schedule::UPDATE);

// Physics phase
app.add_systems<collision, physics_solver>(Schedule::FIXED_UPDATE);

// Render phase
app.add_systems<sprite_render, ui_render>(Schedule::RENDER_2D);
```

### Use Sets for Broad Dependencies

System sets are excellent for establishing high-level ordering between different parts of your application, like ensuring all game logic runs before all physics calculations within the same schedule.

```cpp
struct GameLogicSet {};
struct PhysicsSet {};

app.add_systems<player_logic, enemy_logic>(Schedule::UPDATE)
   .in_set<GameLogicSet>();

app.add_systems<collision_system, solver_system>(Schedule::UPDATE)
   .in_set<PhysicsSet>();

// Configure the entire PhysicsSet to run after the GameLogicSet
app.configure_sets<PhysicsSet>(Schedule::UPDATE)
   .after<GameLogicSet>();
```

## Example: Complete Setup

```cpp
Application{}
    // Startup
    .add_systems<load_assets, spawn_player>(Schedule::STARTUP)

    // Update: Game Logic and Input
    .add_systems<input_system>(Schedule::UPDATE)
    .add_systems<
        player_movement,
        enemy_ai,
        projectile_movement
    >(Schedule::UPDATE)
    .after<input_system>()

    // Fixed Update: Physics
    .add_systems<collision_detection, physics_solver>(Schedule::FIXED_UPDATE)

    // Render
    .add_systems<render_models>(Schedule::RENDER_3D)
    .add_systems<render_ui>(Schedule::RENDER_2D)
    .run();
```

## Next Steps

- See [Examples](../examples/index.md) for complete applications
- Check [API Reference](../api/index.md) for technical details
