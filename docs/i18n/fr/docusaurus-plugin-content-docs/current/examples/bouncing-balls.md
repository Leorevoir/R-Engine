---
sidebar_position: 1
---

# Exemple de Balles Rebondissantes

Une simulation physique simple démontrant les entités, les composants et les systèmes.

## Vue d'Ensemble

Cet exemple crée plusieurs entités de balles qui rebondissent sur l'écran, démontrant :

- La création d'entités
- Les données basées sur les composants
- La logique système pour le mouvement et la collision
- L'itération de requêtes
- L'utilisation de ressources pour le temps de frame

## Composants

```cpp
struct Position {
    Vec2f value;
};

struct Velocity {
    Vec2f value;
};

struct Circle {
    float radius;
    Color color;
};
```

## Systèmes

### Système de Création

Crée les entités de balles une fois au démarrage.

```cpp
void spawn_system(ecs::Commands& commands) {
    for (int i = 0; i < 10; i++) {
        commands.spawn(
            Position{Vec2f{rand() % 800, rand() % 600}},
            Velocity{Vec2f{rand_float(-100, 100), rand_float(-100, 100)}},
            Circle{20.0f, random_color()}
        );
    }
}
```

### Système de Mouvement

Met à jour les positions en fonction de la vélocité à chaque frame.

```cpp
void movement_system(
    ecs::Query<ecs::Mut<Position>, ecs::Ref<Velocity>> query,
    ecs::Res<core::FrameTime> time
) {
    for (auto [pos, vel] : query) {
        pos.ptr->value += vel.ptr->value * time.ptr->delta_time;
    }
}
```

### Système de Rebond

Inverse la vélocité lorsque les balles heurtent les bords de l'écran.

```cpp
void bounce_system(
    ecs::Query<ecs::Ref<Position>, ecs::Mut<Velocity>, ecs::Ref<Circle>> query
) {
    for (auto [pos, vel, circle] : query) {
        // Murs gauche/droite
        if ((pos.ptr->value.x - circle.ptr->radius < 0 && vel.ptr->value.x < 0) ||
            (pos.ptr->value.x + circle.ptr->radius > 800 && vel.ptr->value.x > 0)) {
            vel.ptr->value.x *= -1;
        }

        // Murs haut/bas
        if ((pos.ptr->value.y - circle.ptr->radius < 0 && vel.ptr->value.y < 0) ||
            (pos.ptr->value.y + circle.ptr->radius > 600 && vel.ptr->value.y > 0)) {
            vel.ptr->value.y *= -1;
        }
    }
}
```

### Système de Rendu

Dessine les balles.

```cpp
void render_system(
    ecs::Query<ecs::Ref<Position>, ecs::Ref<Circle>> query
) {
    for (auto [pos, circle] : query) {
        DrawCircle(
            pos.ptr->value.x,
            pos.ptr->value.y,
            circle.ptr->radius,
            circle.ptr->color
        );
    }
}
```

## Configuration de l'Application

```cpp
int main() {
    Application{}
        .add_systems<spawn_system>(Schedule::STARTUP)
        .add_systems<
            movement_system,
            bounce_system
        >(Schedule::UPDATE)
        .add_systems<render_system>(Schedule::RENDER_2D)
        .run();

    return 0;
}
```

## Concepts Clés

1.  **Composition de Composants** : Chaque balle est une `Entity` avec les composants `Position`, `Velocity` et `Circle`.
2.  **Séparation des Systèmes** : La logique de mouvement, de rebond et de rendu est dans des systèmes séparés et focalisés.
3.  **Itération de Requête** : Chaque système itère efficacement uniquement sur les entités qui ont les composants dont il a besoin.
4.  **Utilisation de Ressource** : La ressource `FrameTime` est utilisée par le système de mouvement pour assurer un mouvement indépendant de la fréquence d'images.

## Exécuter l'Exemple

Compilez et exécutez l'exemple depuis votre répertoire de build.

## Prochaines Étapes

- Essayez [Hiérarchie de Systèmes](./system-hierarchy.md) pour les relations parent-enfant.
- Consultez [Communication par Événements](./event-communication.md) pour la messagerie inter-systèmes.
