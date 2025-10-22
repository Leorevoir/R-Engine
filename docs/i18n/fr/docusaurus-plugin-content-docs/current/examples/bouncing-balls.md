---
sidebar_position: 1
---

# Exemple Balles Rebondissantes

Une simulation physique simple démontrant les entités, composants et systèmes.

## Vue d'Ensemble

Cet exemple crée plusieurs entités balles qui rebondissent sur l'écran, démontrant :
- Création d'entités
- Données basées sur les composants
- Logique des systèmes
- Itération de requêtes

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

## Systems

### Système de Création

Crée les entités balles :

```cpp
void spawn_system(Commands& commands) {
    for (int i = 0; i < 10; i++) {
        commands.spawn(
            Position{Vec2f{rand() % 800, rand() % 600}},
            Velocity{Vec2f{rand_float(-5, 5), rand_float(-5, 5)}},
            Circle{20.0f, random_color()}
        );
    }
}
```

### Système de Mouvement

Met à jour les positions basées sur la vélocité :

```cpp
void movement_system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<DeltaTime> time
) {
    for (auto [pos, vel] : query) {
        pos.ptr->value += vel.ptr->value * time.ptr->dt;
    }
}
```

### Système de Rebond

Fait rebondir les balles sur les bords de l'écran :

```cpp
void bounce_system(
    Query<Ref<Position>, Mut<Velocity>, Ref<Circle>> query
) {
    for (auto [pos, vel, circle] : query) {
        // Murs gauche/droite
        if (pos.ptr->value.x - circle.ptr->radius < 0 ||
            pos.ptr->value.x + circle.ptr->radius > 800) {
            vel.ptr->value.x *= -1;
        }
        
        // Murs haut/bas
        if (pos.ptr->value.y - circle.ptr->radius < 0 ||
            pos.ptr->value.y + circle.ptr->radius > 600) {
            vel.ptr->value.y *= -1;
        }
    }
}
```

### Système de Rendu

Dessine les balles :

```cpp
void render_system(
    Query<Ref<Position>, Ref<Circle>> query
) {
    for (auto [pos, circle] : query) {
        draw_circle(
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
        .insert_resource(DeltaTime{})
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

1. **Composition de Composants** : Chaque balle est une entité avec les composants Position, Velocity et Circle
2. **Séparation des Systèmes** : Mouvement, rebond et rendu sont des systèmes séparés
3. **Itération de Requête** : Chaque système itère efficacement sur les entités pertinentes
4. **Utilisation de Ressource** : La ressource DeltaTime fournit le timing des frames

## Exécuter l'Exemple

```bash
cd R-Engine/build
./r-engine__bouncing_balls
```

## Prochaines Étapes

- Essayez [Hiérarchie de Systèmes](./system-hierarchy.md) pour les relations parent-enfant
- Consultez [Communication par Événements](./event-communication.md) pour la messagerie inter-système
