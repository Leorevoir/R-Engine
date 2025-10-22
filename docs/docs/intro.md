---
sidebar_position: 1
slug: /
---

# Introduction to R-Engine ECS

R-Engine uses an **Entity Component System (ECS)** architecture inspired by Bevy, designed to provide performance, flexibility and ease of use.

## What is an ECS?

:::info

An ECS is an architectural design pattern that separates data (Components) from logic (Systems), connecting them through entities (Entities). This approach favors composition over inheritance and enables more modular and performant code.

:::

## Key Features

- **Archetype-based Storage**: Archetype-optimized storage for maximum performance
- **Type Safety**: Compile-time type system to avoid errors
- **Dependency Injection**: Automatic system dependency resolution
- **Command Buffer**: Deferred modifications to avoid iterator invalidation
- **Events System**: Asynchronous communication between systems
- **Hierarchy Support**: Native parent-child relationships

## Why ECS?

Traditional object-oriented architectures with deep inheritance hierarchies can become difficult to maintain and extend. ECS offers several advantages:

### Performance

- **Cache-friendly**: Components are stored contiguously in memory
- **Data-oriented design**: Optimized for modern CPU architectures
- **Parallel execution**: Systems can run in parallel when they don't conflict

### Flexibility

- **Composition over inheritance**: Mix and match components easily
- **Runtime flexibility**: Add/remove components dynamically
- **Modular systems**: Easy to add, remove, or modify behavior

### Maintainability

- **Clear separation of concerns**: Data and logic are separate
- **Easy testing**: Systems can be tested in isolation
- **Reusable components**: Share components across different entity types

## Getting Started

To start using R-Engine ECS in your application:

```cpp
#include "R-Engine/Application.hpp"

using namespace REngine;

int main() {
    Application{}
        .add_systems<startup_system>(Schedule::STARTUP)
        .add_systems<update_system>(Schedule::UPDATE)
        .run();
    
    return 0;
}
```

## Next Steps

- Learn about [Architecture](./architecture.md) to understand how R-Engine ECS works internally
- Explore [Core Concepts](./core-concepts/index.md) to understand entities, components, and systems
- Check out [Examples](./examples/index.md) for practical code samples
