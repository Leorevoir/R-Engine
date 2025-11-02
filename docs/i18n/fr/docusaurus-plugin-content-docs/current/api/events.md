---
sidebar_position: 5
---

# API des Événements

Communication par événements entre les systèmes.

## EventWriter\<T\>

Un paramètre de système pour envoyer des événements à d'autres systèmes.

### send()

```cpp
void send(const EventT& event);
void send(EventT&& event);
```

Envoie un événement de type `T`.

**Exemple** :

```cpp
void system(ecs::EventWriter<CollisionEvent> events) {
    events.send(CollisionEvent{entity_a, entity_b});
}
```

## EventReader\<T\>

Un paramètre de système pour recevoir des événements d'autres systèmes.

### Itération

`EventReader` est directement itérable.

```cpp
void system(ecs::EventReader<CollisionEvent> events) {
    for (const auto& event : events) {
        // Gérer l'événement
        std::cout << "Collision!\n";
    }
}
```

### has_events()

```cpp
bool has_events() const noexcept;
```

Vérifie s'il y a des événements à lire. Utile pour les conditions d'exécution.

## Cycle de Vie des Événements

Les événements sont gérés avec un système de **double-tampon**. Les événements envoyés dans la frame `N` sont disponibles pour être lus par n'importe quel système dans la **frame `N+1`**. À la fin de la frame `N+1`, pendant le schedule `EVENT_CLEANUP`, le tampon qui a été lu est vidé.

```
Frame N:
  - EventWriter envoie l'événement E1.
  - L'événement E1 est stocké dans le tampon "d'écriture".
  - Pendant EVENT_CLEANUP, les tampons sont échangés.

Frame N+1:
  - EventReader(s) peut maintenant lire l'événement E1 depuis le tampon "de lecture".
  - EventWriter envoie de nouveaux événements au tampon "d'écriture".
  - Pendant EVENT_CLEANUP, le tampon "de lecture" (avec E1) est vidé, et les tampons sont à nouveau échangés.
```

## Pattern d'Utilisation

```cpp
// 1. Définir l'événement
struct DamageEvent {
    ecs::Entity target;
    int amount;
};

// 2. Enregistrer l'événement dans l'application
Application{}
    .add_events<DamageEvent>()
    // ...

// 3. Envoyer des événements depuis un système
void combat_system(ecs::EventWriter<DamageEvent> events) {
    events.send(DamageEvent{enemy, 25});
}

// 4. Lire des événements dans un autre système
void health_system(
    ecs::EventReader<DamageEvent> events,
    ecs::Query<ecs::Mut<Health>> query
) {
    for (const auto& event : events) {
        if (auto* health = query.get_component_ptr<Health>(event.target)) {
            health->current -= event.amount;
        }
    }
}
```

## Lecteurs Multiples

Plusieurs systèmes peuvent avoir un `EventReader` pour le même type d'événement. Tous recevront tous les événements envoyés cette frame-là.

```cpp
// Système 1
void damage_system(ecs::EventReader<CollisionEvent> events) {
    for (const auto& e : events) {
        // Appliquer les dégâts
    }
}

// Système 2
void sound_system(ecs::EventReader<CollisionEvent> events) {
    for (const auto& e : events) {
        // Jouer un son
    }
}
```

## Voir Aussi

- [Guide des Événements](../advanced/events.md)
- [Exemple de Communication par Événements](../examples/event-communication.md)
