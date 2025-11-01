---
sidebar_position: 9
---

# Référence API

Référence API complète pour R-Engine ECS.

## Classes Principales

### Scene

Le monde ECS principal qui gère les entités, composants et ressources.

[API Scene →](./scene.md)

### Commands

Tampon de commandes différé pour modifier le monde ECS.

[API Commands →](./commands.md)

### EntityCommands

Commandes pour une entité spécifique.

[API EntityCommands →](./entity-commands.md)

### Query avec Wrappers

Itération d'entités type-safe avec accès aux composants.

[API Query →](./query.md)

## Système d'Événements

### EventWriter\<T\>

Envoyer des événements vers d'autres systèmes.

### EventReader\<T\>

Recevoir des événements depuis d'autres systèmes.

[API Events →](./events.md)

## Référence Rapide

| Type | Objectif | Exemple |
|------|---------|---------|
| `Entity` | Identifiant d'entité | `Entity player = ...;` |
| `Ref<T>` | Accès composant lecture seule | `Ref<Position>` |
| `Mut<T>` | Accès composant mutable | `Mut<Position>` |
| `With<T>` | Filtre : possède le composant | `With<Player>` |
| `Without<T>` | Filtre : n'a pas le composant | `Without<Dead>` |
| `Res<T>` | Ressource lecture seule | `Res<Config>` |
| `ResMut<T>` | Ressource mutable | `ResMut<Score>` |
| `Commands` | Tampon de commandes | `Commands& cmds` |
| `EventWriter<T>` | Envoyer événements | `EventWriter<E>` |
| `EventReader<T>` | Lire événements | `EventReader<E>` |

## Patterns d'Utilisation

### Signature de Système

```cpp
void system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<Time> time,
    Commands& commands,
    EventWriter<Event> events
) {
    // Logique du système
}
```

### Itération de Requête

```cpp
for (auto [pos, vel] : query) {
    pos.ptr->x += vel.ptr->x;
}
```

### Utilisation de Commandes

```cpp
commands.spawn(Position{}, Velocity{});
commands.entity(e).insert(Health{100});
commands.insert_resource(Config{});
```

## Prochaines Étapes

Parcourez la documentation API détaillée pour chaque classe.
