---
sidebar_position: 8
---

# Examples

Practical examples demonstrating R-Engine ECS features.

## Overview

These examples show real-world usage of R-Engine ECS:

### Bouncing Balls

A simple physics simulation with multiple entities.

[View Bouncing Balls Example →](./bouncing-balls.md)

### System Hierarchy

Demonstrates parent-child entity relationships.

[View System Hierarchy Example →](./system-hierarchy.md)

### Event Communication

Shows inter-system communication using events.

[View Event Communication Example →](./event-communication.md)

## Running Examples

All examples are in the `examples/` directory:

```bash
# Build examples
cd R-Engine
mkdir build && cd build
cmake ..
make

# Run an example
./r-engine__bouncing_balls
./r-engine__system_hierarchy
./r-engine__event_writer_reader
```

## Example Structure

Each example follows this pattern:

```cpp
#include "R-Engine/Application.hpp"

// 1. Define components
struct Position { float x, y; };
struct Velocity { float x, y; };

// 2. Define systems
void movement_system(Query<Mut<Position>, Ref<Velocity>> q) {
    for (auto [pos, vel] : q) {
        pos->x += vel->x;
        pos->y += vel->y;
    }
}

// 3. Setup application
int main() {
    Application{}
        .add_systems<movement_system>(Schedule::UPDATE)
        .run();
    return 0;
}
```

## Next Steps

Start with [Bouncing Balls](./bouncing-balls.md) for a simple example.
