---
sidebar_position: 3
---

# Exemple Communication par Événements

Démontre la communication inter-système utilisant les événements.

## Vue d'Ensemble

Cet exemple montre comment les systèmes communiquent sans couplage fort en utilisant les événements :
- L'entrée joueur génère des événements
- Le système de création de balles réagit aux événements
- Le système de collision envoie des événements de dégâts
- Le système de santé reçoit les événements de dégâts

## Définitions d'Événements

```cpp
struct FireEvent {
    Entity shooter;
    Vec2f position;
    Vec2f direction;
};

struct DamageEvent {
    Entity target;
    int damage;
};

struct DeathEvent {
    Entity entity;
    int score_value;
};
```

## Systems

### Système d'Entrée

Envoie des événements de tir :

```cpp
void input_system(
    Query<Entity, Ref<Position>, With<Player>> query,
    Res<Input> input,
    EventWriter<FireEvent> events
) {
    if (input.ptr->fire_pressed) {
        for (auto [entity, pos, _] : query) {
            events.send(FireEvent{
                entity,
                pos.ptr->value,
                Vec2f{1, 0}  // Tirer vers la droite
            });
        }
    }
}
```

### Système de Création de Balles

Réagit aux événements de tir :

```cpp
void spawn_bullets(
    EventReader<FireEvent> events,
    Commands& commands
) {
    for (const auto& event : events.iter()) {
        commands.spawn(
            Position{event.position},
            Velocity{event.direction * 500.0f},
            Bullet{event.shooter}
        );
    }
}
```

### Système de Collision

Détecte les collisions et envoie des événements de dégâts :

```cpp
void collision_system(
    Query<Entity, Ref<Position>, With<Bullet>> bullets,
    Query<Entity, Ref<Position>, With<Enemy>> enemies,
    EventWriter<DamageEvent> events,
    Commands& commands
) {
    for (auto [bullet_entity, bullet_pos, _] : bullets) {
        for (auto [enemy_entity, enemy_pos, _] : enemies) {
            if (distance(bullet_pos.ptr->value, enemy_pos.ptr->value) < 20.0f) {
                // Envoyer l'événement de dégâts
                events.send(DamageEvent{enemy_entity, 25});
                
                // Détruire la balle
                commands.entity(bullet_entity).despawn();
            }
        }
    }
}
```

### Système de Santé

Reçoit les événements de dégâts :

```cpp
void health_system(
    EventReader<DamageEvent> damage_events,
    EventWriter<DeathEvent> death_events,
    Query<Entity, Mut<Health>> query,
    Commands& commands
) {
    for (const auto& event : damage_events.iter()) {
        if (auto health = query.get<Mut<Health>>(event.target)) {
            health.ptr->current -= event.damage;
            
            // Vérifier la mort
            if (health.ptr->current <= 0) {
                death_events.send(DeathEvent{event.target, 100});
                commands.entity(event.target).despawn();
            }
        }
    }
}
```

### Système de Score

Suit le score depuis les événements de mort :

```cpp
void score_system(
    EventReader<DeathEvent> events,
    ResMut<Score> score
) {
    for (const auto& event : events.iter()) {
        score.ptr->value += event.score_value;
        std::cout << "Score: " << score.ptr->value << "\n";
    }
}
```

## Configuration de l'Application

```cpp
int main() {
    Application{}
        // Enregistrer les événements
        .add_event<FireEvent>()
        .add_event<DamageEvent>()
        .add_event<DeathEvent>()
        
        // Ressources
        .insert_resource(Input{})
        .insert_resource(Score{0})
        
        // Systèmes
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

## Flux d'Événements

```
Entrée Joueur
    ↓ (FireEvent)
Créer des Balles
    ↓ (les balles se déplacent)
Détection de Collision
    ↓ (DamageEvent)
Système de Santé
    ↓ (DeathEvent)
Système de Score
```

## Concepts Clés

1. **Couplage Lâche** : Les systèmes ne s'appellent pas directement
2. **Orienté Événements** : Les actions déclenchent des événements auxquels d'autres peuvent réagir
3. **Récepteurs Multiples** : Plusieurs systèmes peuvent lire le même événement
4. **Durée d'une Frame** : Les événements sont effacés chaque frame

## Exécution

```bash
./r-engine__event_writer_reader
```

## Prochaines Étapes

- En savoir plus sur les [Événements](../advanced/events.md)
- Consultez la [Référence API](../api/events.md)
