---
sidebar_position: 5
---

# API des événements

Communication par événements entre systèmes.

## EventWriter\<T\>

Envoyer des événements vers d'autres systèmes.

### send()

```cpp
void send(T event)
```

Envoyer un événement à diffuser.

**Exemple** :
```cpp
void system(EventWriter<CollisionEvent> events) {
    events.send(CollisionEvent{entity_a, entity_b});
}
```

## EventReader\<T\>

Recevoir des événements depuis d'autres systèmes.

### iter()

```cpp
Iterator iter()
```

Itérer sur tous les événements envoyés cette frame.

**Exemple** :
```cpp
void system(EventReader<CollisionEvent> events) {
    for (const auto& event : events.iter()) {
        // Gérer l'événement
        std::cout << "Collision!\n";
    }
}
```

## Cycle de Vie des Événements

Les événements durent une frame :

```
Frame N :
  - EventWriter envoie un événement
  - EventReader(s) lit l'événement
  - L'événement est effacé en fin de frame

Frame N+1 :
  - L'événement n'est plus disponible
```

## Pattern d'Utilisation

```cpp
// 1. Définir l'événement
struct DamageEvent {
    Entity target;
    int amount;
};

// 2. Enregistrer l'événement
Application{}
    .add_event<DamageEvent>()
    // ...

// 3. Envoyer des événements
void combat_system(EventWriter<DamageEvent> events) {
    events.send(DamageEvent{enemy, 25});
}

// 4. Lire des événements
void health_system(
    EventReader<DamageEvent> events,
    Query<Mut<Health>> query
) {
    for (const auto& event : events.iter()) {
        if (auto health = query.get<Mut<Health>>(event.target)) {
            health.ptr->current -= event.amount;
        }
    }
}
```

## Lecteurs Multiples

Plusieurs systèmes peuvent lire le même événement :

```cpp
// Système 1
void damage_system(EventReader<CollisionEvent> events) {
    for (const auto& e : events.iter()) {
        // Appliquer les dégâts
    }
}

// Système 2
void sound_system(EventReader<CollisionEvent> events) {
    for (const auto& e : events.iter()) {
        // Jouer un son
    }
}
```

## Voir Aussi

- [Guide Events](../advanced/events.md)
- [Exemple Communication par Événements](../examples/event-communication.md)
