---
sidebar_position: 1
---

# Événements

Les événements fournissent une communication asynchrone et découplée entre les systèmes.

## Que sont les Événements ?

Les événements sont des messages envoyés par un système et reçus par d'autres, permettant un couplage lâche entre les systèmes de jeu.

```cpp
// Définir un événement
struct CollisionEvent {
    ecs::Entity entity_a;
    ecs::Entity entity_b;
    Vec2f impact_point;
};
```

## Enregistrer les Événements

Ajoutez des événements à votre application en utilisant la méthode `add_events` :

```cpp
Application{}
    .add_events<CollisionEvent, DamageEvent>()
    .run();
```

## Envoyer des Événements

Utilisez `ecs::EventWriter<T>` pour envoyer des événements :

```cpp
void collision_system(
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::Ref<Collider>> query,
    ecs::EventWriter<CollisionEvent> events
) {
    // Vérifier les collisions...
    if (collision_detected) {
        events.send(CollisionEvent{
            .entity_a = e1,
            .entity_b = e2,
            .impact_point = point
        });
    }
}
```

## Lire les Événements

Utilisez `ecs::EventReader<T>` pour recevoir des événements. Le lecteur est directement itérable.

```cpp
void damage_system(
    ecs::EventReader<CollisionEvent> collision_events,
    ecs::Query<ecs::Mut<Health>> query
) {
    for (const auto& event : collision_events) {
        // Gérer l'événement de collision
        if (auto health = query.get_component_ptr<Health>(event.entity_a)) {
            health->value -= 10;
        }
    }
}
```

## Cycle de Vie des Événements

Les événements sont gérés avec un système de **double-tampon** (double-buffer). Cela signifie que les événements envoyés dans une frame donnée ne sont disponibles pour la lecture que dans la **frame suivante**.

```
Frame N:
Système A envoie l'événement E1
Système B envoie l'événement E2
...
(Les événements E1 et E2 sont stockés dans le tampon "d'écriture" pour cette frame)
...
Le schedule EVENT_CLEANUP s'exécute
Les tampons d'événements sont échangés. Le tampon "d'écriture" devient le tampon "de lecture".

Frame N+1:
Le système C lit les événements E1 et E2 depuis le tampon "de lecture".
...
Le schedule EVENT_CLEANUP s'exécute
Le tampon "de lecture" (contenant E1, E2) est vidé, et les tampons sont à nouveau échangés.
```

## Patterns d'Événements Courants

### Événements d'Entrée

```cpp
struct KeyPressEvent {
    int key_code;
    bool pressed;
};

void input_system(ecs::EventWriter<KeyPressEvent> events) {
    // Interroger les entrées
    if (IsKeyPressed(KEY_SPACE)) {
        events.send(KeyPressEvent{KEY_SPACE, true});
    }
}

void player_system(ecs::EventReader<KeyPressEvent> events) {
    for (const auto& event : events) {
        if (event.key_code == KEY_SPACE && event.pressed) {
            // Sauter !
        }
    }
}
```

### Événements de Jeu

```cpp
struct EnemyDefeatedEvent {
    ecs::Entity enemy;
    int score_value;
};

void combat_system(
    ecs::Query<ecs::Entity, ecs::Mut<Health>, ecs::With<Enemy>> query,
    ecs::EventWriter<EnemyDefeatedEvent> events,
    ecs::Commands& commands
) {
    for (auto [entity, health, _] : query) {
        if (health.ptr->current <= 0) {
            events.send(EnemyDefeatedEvent{entity, 100});
            commands.despawn(entity);
        }
    }
}

void score_system(
    ecs::EventReader<EnemyDefeatedEvent> events,
    ecs::ResMut<Score> score
) {
    for (const auto& event : events) {
        score.ptr->value += event.score_value;
    }
}
```

## Bonnes Pratiques

### ✅ À Faire

- Utilisez les événements pour un couplage lâche entre systèmes
- Gardez les données d'événement légères
- Utilisez des noms d'événements descriptifs

### ❌ À Ne Pas Faire

- N'utilisez pas les événements pour chaque interaction système (les requêtes sont bien pour l'accès direct)
- Ne stockez pas de grandes quantités de données dans les événements
- N'attendez pas que les événements soient lisibles dans la même frame où ils sont envoyés.

## Exemple : Flux d'Événements Complet

```cpp
// Définitions d'événements
struct BulletFiredEvent {
    ecs::Entity shooter;
    Vec2f position;
    Vec2f direction;
};

struct HitEvent {
    ecs::Entity target;
    int damage;
};

// Système 1 : Tirer des balles
void shooting_system(
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::With<Player>> query,
    ecs::Res<UserInput> input,
    ecs::EventWriter<BulletFiredEvent> events
) {
    if (input.ptr->isKeyJustPressed(KEY_SPACE)) {
        for (auto it = query.begin(); it != query.end(); ++it) {
            auto [pos, _] = *it;
            events.send(BulletFiredEvent{
                it.entity(), pos.ptr->value, Vec2f{1, 0}
            });
        }
    }
}

// Système 2 : Créer des balles à partir des événements
void bullet_spawn_system(
    ecs::EventReader<BulletFiredEvent> events,
    ecs::Commands& commands
) {
    for (const auto& event : events) {
        commands.spawn(
            Bullet{},
            Position{event.position},
            Velocity{event.direction * 10.0f}
        );
    }
}

// Système 3 : Détecter les impacts
void collision_system(
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::With<Bullet>> bullets,
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::With<Enemy>> enemies,
    ecs::EventWriter<HitEvent> events
) {
    for (auto it_bullet = bullets.begin(); it_bullet != bullets.end(); ++it_bullet) {
        auto [bullet_pos, _] = *it_bullet;
        for (auto it_enemy = enemies.begin(); it_enemy != enemies.end(); ++it_enemy) {
            auto [enemy_pos, _] = *it_enemy;
            if (distance(bullet_pos.ptr->value, enemy_pos.ptr->value) < 10.0f) {
                events.send(HitEvent{it_enemy.entity(), 25});
            }
        }
    }
}

// Système 4 : Appliquer les dégâts
void damage_system(
    ecs::EventReader<HitEvent> events,
    ecs::Query<ecs::Mut<Health>> query
) {
    for (const auto& event : events) {
        if (auto* health = query.get_component_ptr<Health>(event.target)) {
            health->current -= event.damage;
        }
    }
}

// Application
Application{}
    .add_events<BulletFiredEvent, HitEvent>()
    .add_systems<
        shooting_system,
        bullet_spawn_system,
        collision_system,
        damage_system
    >(Schedule::UPDATE)
    .run();
```

## Prochaines Étapes

- Apprenez-en plus sur les [Hiérarchies](./hierarchies.md)
- Consultez les [Exemples](../examples/event-communication.md)
