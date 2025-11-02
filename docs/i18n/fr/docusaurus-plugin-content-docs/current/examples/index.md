---
sidebar_position: 8
---

# Exemples

Exemples pratiques démontrant les fonctionnalités de R-Engine ECS.

## Vue d'Ensemble

Ces exemples montrent l'utilisation réelle de R-Engine ECS :

### Balles Rebondissantes

Une simulation physique simple avec plusieurs entités.

[Voir l'Exemple des Balles Rebondissantes →](./bouncing-balls.md)

### Hiérarchie de Systèmes

Démontre les relations parent-enfant entre entités.

[Voir l'Exemple de Hiérarchie de Systèmes →](./system-hierarchy.md)

### Communication par Événements

Montre la communication inter-système à l'aide d'événements.

[Voir l'Exemple de Communication par Événements →](./event-communication.md)

## Exécuter les Exemples

Tous les exemples sont dans le répertoire `examples/` :

```bash
# Compiler les exemples
cd R-Engine
mkdir build && cd build
cmake ..
make

# Exécuter un exemple
./r-engine__bouncing_balls
./r-engine__system_hierarchy
./r-engine__event_writer_reader
```

## Structure d'un Exemple

Chaque exemple suit ce modèle :

```cpp
#include "R-Engine/Application.hpp"

// 1. Définir les composants
struct Position { float x, y; };
struct Velocity { float x, y; };

// 2. Définir les systèmes
void movement_system(Query<Mut<Position>, Ref<Velocity>> q) {
    for (auto [pos, vel] : q) {
        pos->x += vel->x;
        pos->y += vel->y;
    }
}

// 3. Configurer l'application
int main() {
    Application{}
        .add_systems<movement_system>(Schedule::UPDATE)
        .run();
    return 0;
}
```

## Prochaines Étapes

Commencez avec [Balles Rebondissantes](./bouncing-balls.md) pour un exemple simple.
