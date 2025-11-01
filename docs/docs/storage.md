---
sidebar_position: 7
---

# Storage System

R-Engine uses an **archetype-based storage system** for optimal performance and memory efficiency.

## Archetype Overview

An archetype is a unique combination of component types. Entities with the same set of components share the same archetype.

```
Archetype A: [Position, Velocity]
  Entity 1: Position{0,0}, Velocity{1,0}
  Entity 2: Position{5,3}, Velocity{-1,2}

Archetype B: [Position, Velocity, Health]
  Entity 10: Position{2,1}, Velocity{0,1}, Health{100}
```

## Memory Layout

Components are stored in contiguous arrays per archetype:

```
Archetype [Position, Velocity]
┌──────────────────────────────┐
│ Positions: [P1][P2][P3][P4]  │
│ Velocities: [V1][V2][V3][V4] │
│ Entity IDs: [E1][E2][E3][E4] │
└──────────────────────────────┘
```

## Benefits

### Cache-Friendly

Components of the same type are adjacent in memory, improving CPU cache utilization.

### Fast Iteration

Queries iterate over entire archetypes at once, processing multiple entities efficiently.

### Memory Efficient

No pointer indirection - direct access to component data.

## Archetype Changes

When components are added or removed, entities move between archetypes:

```cpp
// Entity in archetype [Position, Velocity]
Entity e = commands.spawn(Position{}, Velocity{}).id();

// Add Health → moves to archetype [Position, Velocity, Health]
commands.entity(e).insert(Health{100});

// Remove Velocity → moves to archetype [Position, Health]
commands.entity(e).remove<Velocity>();
```

:::tip Performance
Minimize archetype changes during performance-critical code. Each change requires moving component data.
:::

## Query Matching

Queries automatically match all compatible archetypes:

```cpp
// Query: Position + Velocity
Query<Mut<Position>, Ref<Velocity>> query;

// Matches:
// ✓ [Position, Velocity]
// ✓ [Position, Velocity, Health]
// ✓ [Position, Velocity, Sprite, AI]
//
// Does NOT match:
// ✗ [Position]
// ✗ [Velocity]
// ✗ [Position, Health]
```

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| Component Access | O(1) | Direct array access |
| Query Iteration | O(n) | Linear in entity count |
| Add Component | O(n_components) | Copy components to new archetype |
| Remove Component | O(n_components) | Copy to new archetype |
| Spawn Entity | O(1) amortized | Append to archetype |
| Despawn Entity | O(1) amortized | Swap-remove from archetype |

## Best Practices

### ✅ Do

- Design components to minimize archetype changes
- Use marker components for categorization
- Batch spawn operations

### ❌ Don't

- Don't add/remove components in tight loops
- Don't create too many unique component combinations
- Don't frequently change entity composition

## Example: Efficient Design

```cpp
// Good: Stable component composition
struct Unit {
    int health;
    bool is_dead;  // ✓ Flag instead of adding/removing Dead component
};

// Less efficient: Frequent archetype changes
void bad_pattern(Entity e, Commands& commands) {
    if (health <= 0) {
        commands.entity(e).insert(Dead{});  // ✗ Causes archetype change
    } else {
        commands.entity(e).remove<Dead>();  // ✗ Another change
    }
}

// Better: Use flag
void good_pattern(Query<Mut<Unit>> query) {
    for (auto [unit] : query) {
        unit->is_dead = (unit->health <= 0);  // ✓ No archetype change
    }
}
```

## Next Steps

- Explore [Examples](./examples/index.md) to see archetype usage
- Check [API Reference](./api/index.md) for technical details
