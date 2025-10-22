---
sidebar_position: 4
---

# Schedules

Schedules organize system execution into distinct phases.

## Built-in Schedules

R-Engine provides several default schedules:

```cpp
Schedule::STARTUP    // Run once at application start
Schedule::PRE_UPDATE // Before main update
Schedule::UPDATE     // Main game loop
Schedule::POST_UPDATE // After update
Schedule::RENDER_2D  // 2D rendering
Schedule::RENDER_3D  // 3D rendering
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

### Within Schedule

```cpp
.add_systems<input, movement, collision>(Schedule::UPDATE)
.after<input>()  // movement runs after input
```

### Between Schedules

Schedules run in predefined order:

```
STARTUP (once)
↓
PRE_UPDATE
↓
UPDATE
↓
POST_UPDATE
↓
RENDER_2D
↓
RENDER_3D
↓
(repeat from PRE_UPDATE)
```

## System Sets

Group related systems:

```cpp
enum class GameSets {
    Input,
    Logic,
    Physics,
    Rendering
};

.add_systems<input_system>(Schedule::UPDATE)
.in_set(GameSets::Input)

.add_systems<movement, ai>(Schedule::UPDATE)
.in_set(GameSets::Logic)
.after_set(GameSets::Input)

.add_systems<physics>(Schedule::UPDATE)
.in_set(GameSets::Physics)
.after_set(GameSets::Logic)
```

## Best Practices

### Organize by Phase

```cpp
// Input phase
.add_systems<keyboard_input, mouse_input>(Schedule::PRE_UPDATE)

// Logic phase
.add_systems<player_movement, enemy_ai>(Schedule::UPDATE)

// Physics phase
.add_systems<collision, physics>(Schedule::POST_UPDATE)

// Render phase
.add_systems<sprite_render, ui_render>(Schedule::RENDER_2D)
```

### Use Sets for Dependencies

```cpp
.add_systems<game_logic_a, game_logic_b>(Schedule::UPDATE)
.in_set(GameSets::Logic)

.add_systems<render_a, render_b>(Schedule::RENDER_2D)
.in_set(GameSets::Rendering)
.after_set(GameSets::Logic)
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
    .add_systems<collision_detection, physics>(Schedule::POST_UPDATE)
    
    // Render
    .add_systems<render_sprites, render_ui>(Schedule::RENDER_2D)
    .run();
```

## Next Steps

- See [Examples](../examples/index.md) for complete applications
- Check [API Reference](../api/index.md) for technical details
