---
sidebar_position: 3
---

# Exemple de Communication par Événements

Démontre la communication inter-système à l'aide d'événements.

## Vue d'Ensemble

Cet exemple montre comment les systèmes peuvent communiquer sans être fortement couplés.

- Un système d'entrée envoie un `FireEvent`.
- Un système de création de balles écoute les `FireEvent` et crée des balles.
- Un système de collision détecte les impacts et envoie un `DamageEvent`.
- Un système de santé écoute les `DamageEvent` et envoie un `DeathEvent` si la santé est épuisée.
- Un système de score écoute les `DeathEvent` pour mettre à jour le score.

## Définitions d'Événements

```cpp
struct FireEvent {
    ecs::Entity shooter;
    Vec2f position;
    Vec2f direction;
};

struct DamageEvent {
    ecs::Entity target;
    int damage;
};

struct DeathEvent {
    ecs::Entity entity;
    int score_value;
};
```

## Systèmes

### Système d'Entrée

Envoie un `FireEvent` lorsque le joueur appuie sur une touche.

```cpp
void input_system(
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::With<Player>> query,
    ecs::Res<UserInput> input,
    ecs::EventWriter<FireEvent> events
) {
    if (input.ptr->isKeyJustPressed(KEY_SPACE)) {
        for (auto it = query.begin(); it != query.end(); ++it) {
            auto [pos, _] = *it;
            events.send(FireEvent{
                it.entity(),
                pos.ptr->value,
                Vec2f{1, 0}  // Tirer vers la droite
            });
        }
    }
}
```

### Système de Création de Balles

Réagit aux `FireEvent` en créant des entités de balles.

```cpp
void spawn_bullets(
    ecs::EventReader<FireEvent> events,
    ecs::Commands& commands
) {
    for (const auto& event : events) {
        commands.spawn(
            Position{event.position},
            Velocity{event.direction * 500.0f},
            Bullet{event.shooter}
        );
    }
}
```

### Système de Collision

Détecte les collisions et envoie un `DamageEvent`.

```cpp
void collision_system(
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::With<Bullet>> bullets,
    ecs::Query<ecs::Entity, ecs::Ref<Position>, ecs::With<Enemy>> enemies,
    ecs::EventWriter<DamageEvent> events,
    ecs::Commands& commands
) {
    for (auto it_bullet = bullets.begin(); it_bullet != bullets.end(); ++it_bullet) {
        auto [bullet_pos, _] = *it_bullet;
        for (auto it_enemy = enemies.begin(); it_enemy != enemies.end(); ++it_enemy) {
            auto [enemy_pos, __] = *it_enemy;
            if (distance(bullet_pos.ptr->value, enemy_pos.ptr->value) < 20.0f) {
                events.send(DamageEvent{it_enemy.entity(), 25});
                commands.despawn(it_bullet.entity());
            }
        }
    }
}
```

### Système de Santé

Reçoit les `DamageEvent` et envoie les `DeathEvent`.

```cpp
void health_system(
    ecs::EventReader<DamageEvent> damage_events,
    ecs::EventWriter<DeathEvent> death_events,
    ecs::Query<ecs::Mut<Health>> query,
    ecs::Commands& commands
) {
    for (const auto& event : damage_events) {
        if (auto* health = query.get_component_ptr<Health>(event.target)) {
            health->current -= event.damage;

            if (health->current <= 0) {
                death_events.send(DeathEvent{event.target, 100});
                commands.despawn(event.target);
            }
        }
    }
}
```

### Système de Score

Suit le score à partir des `DeathEvent`.

```cpp
void score_system(
    ecs::EventReader<DeathEvent> events,
    ecs::ResMut<Score> score
) {
    for (const auto& event : events) {
        score.ptr->value += event.score_value;
        std::cout << "Score: " << score.ptr->value << "\n";
    }
}
```

## Configuration de l'Application

```cpp
int main() {
    Application{}
        // Enregistrer tous les types d'événements
        .add_events<FireEvent, DamageEvent, DeathEvent>()

        // Ajouter des ressources
        .insert_resource(UserInput{})
        .insert_resource(Score{0})

        // Ajouter des systèmes
        .add_systems<
            input_system,
            spawn_bullets,
            collision_system,
            health_system,
            score_system
        >(Schedule::UPDATE)
        .run();

    return 0;
}
```

## Concepts Clés

1.  **Couplage Lâche** : Les systèmes communiquent sans avoir besoin de se connaître mutuellement.
2.  **Logique Pilotée par les Événements** : Les actions déclenchent des événements auxquels d'autres systèmes réagissent.
3.  **Récepteurs Multiples** : Un événement peut être lu par n'importe quel nombre de systèmes.
4.  **Cycle de Vie d'une Frame Suivante** : Les événements envoyés dans une frame sont lisibles dans la suivante.

## Prochaines Étapes

- Apprenez-en plus sur les [Événements](../advanced/events.md)
- Consultez la [Référence API](../api/events.md)
