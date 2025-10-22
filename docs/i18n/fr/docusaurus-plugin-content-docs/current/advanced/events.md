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
    Entity entity_a;
    Entity entity_b;
    Vec2 impact_point;
};
```

## Enregistrer les Events

Ajoutez des événements à votre application :

```cpp
Application{}
    .add_event<CollisionEvent>()
    .add_event<DamageEvent>()
    .run();
```

## Envoyer des Events

Utilisez `EventWriter<T>` pour envoyer des événements :

```cpp
void collision_system(
    Query<Entity, Ref<Position>, Ref<Collider>> query,
    EventWriter<CollisionEvent> events
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

## Lire les Events

Utilisez `EventReader<T>` pour recevoir des événements :

```cpp
void damage_system(
    EventReader<CollisionEvent> events,
    Query<Mut<Health>> query
) {
    for (const auto& event : events.iter()) {
        // Gérer l'événement de collision
        auto health = query.get<Mut<Health>>(event.entity_a);
        if (health) {
            health->value -= 10;
        }
    }
}
```

## Cycle de Vie des Events

Les événements sont effacés après que tous les lecteurs les ont traités :

```
Frame N :
  Le système A envoie un événement
  Le système B lit l'événement
  Le système C lit l'événement
  L'événement est effacé à la fin de la frame

Frame N+1 :
  L'événement n'est plus disponible
```

## Patterns d'Events Courants

### Événements d'Entrée

```cpp
struct KeyPressEvent {
    int key_code;
    bool pressed;
};

void input_system(EventWriter<KeyPressEvent> events) {
    // Interroger les entrées
    if (key_pressed(KEY_SPACE)) {
        events.send(KeyPressEvent{KEY_SPACE, true});
    }
}

void player_system(EventReader<KeyPressEvent> events) {
    for (const auto& event : events.iter()) {
        if (event.key_code == KEY_SPACE && event.pressed) {
            // Sauter !
        }
    }
}
```

### Événements de Jeu

```cpp
struct EnemyDefeatedEvent {
    Entity enemy;
    int score_value;
};

void combat_system(
    Query<Entity, Mut<Health>, With<Enemy>> query,
    EventWriter<EnemyDefeatedEvent> events,
    Commands& commands
) {
    for (auto [entity, health, _] : query) {
        if (health->current <= 0) {
            events.send(EnemyDefeatedEvent{entity, 100});
            commands.entity(entity).despawn();
        }
    }
}

void score_system(
    EventReader<EnemyDefeatedEvent> events,
    ResMut<Score> score
) {
    for (const auto& event : events.iter()) {
        score->value += event.score_value;
    }
}
```

## Bonnes Pratiques

### ✅ Do

- Utilisez les événements pour le couplage lâche entre systèmes
- Gardez les données d'événement légères
- Utilisez des noms d'événements descriptifs

### ❌ Don't

- N'utilisez pas les événements pour chaque interaction système (les requêtes sont bien pour l'accès direct)
- Ne stockez pas de grandes données dans les événements
- N'attendez pas que les événements persistent entre les frames

## Exemple : Flux d'Events Complet

```cpp
// Définitions d'événements
struct BulletFiredEvent {
    Entity shooter;
    Vec2 position;
    Vec2 direction;
};

struct HitEvent {
    Entity target;
    int damage;
};

// Système 1 : Tirer des balles
void shooting_system(
    Query<Entity, Ref<Position>, With<Player>> query,
    Res<Input> input,
    EventWriter<BulletFiredEvent> events
) {
    if (input->fire_pressed) {
        for (auto [entity, pos, _] : query) {
            events.send(BulletFiredEvent{
                entity, pos->value, Vec2{1, 0}
            });
        }
    }
}

// Système 2 : Créer des balles depuis les événements
void bullet_spawn_system(
    EventReader<BulletFiredEvent> events,
    Commands& commands
) {
    for (const auto& event : events.iter()) {
        commands.spawn(
            Bullet{},
            Position{event.position},
            Velocity{event.direction * 10.0f}
        );
    }
}

// Système 3 : Détecter les impacts
void collision_system(
    Query<Entity, Ref<Position>, With<Bullet>> bullets,
    Query<Entity, Ref<Position>, With<Enemy>> enemies,
    EventWriter<HitEvent> events
) {
    for (auto [bullet_e, bullet_pos, _] : bullets) {
        for (auto [enemy_e, enemy_pos, _] : enemies) {
            if (distance(bullet_pos->value, enemy_pos->value) < 10.0f) {
                events.send(HitEvent{enemy_e, 25});
            }
        }
    }
}

// Système 4 : Appliquer les dégâts
void damage_system(
    EventReader<HitEvent> events,
    Query<Mut<Health>> query
) {
    for (const auto& event : events.iter()) {
        if (auto health = query.get<Mut<Health>>(event.target)) {
            health->current -= event.damage;
        }
    }
}

// Application
Application{}
    .add_event<BulletFiredEvent>()
    .add_event<HitEvent>()
    .add_systems<
        shooting_system,
        bullet_spawn_system,
        collision_system,
        damage_system
    >(Schedule::UPDATE)
    .run();
```

## Prochaines Étapes

- Apprenez les [Hierarchies](./hierarchies.md)
- Consultez les [Exemples](../examples/event-communication.md)
