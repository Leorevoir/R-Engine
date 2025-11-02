---
sidebar_position: 2
---

# API des Commandes

`ecs::Commands` fournit des modifications différées au monde ECS.

## Méthodes

### spawn()

```cpp
EntityCommands spawn() noexcept;
EntityCommands spawn(Components&&... components) noexcept;
```

Planifie la création d'une entité. Vous pouvez optionnellement fournir des composants initiaux.

**Retourne** : `EntityCommands` pour une configuration ultérieure.

### entity()

```cpp
EntityCommands entity(Entity e) noexcept;
```

Obtient un handle `EntityCommands` pour une entité existante afin de la modifier.

### insert_resource()

```cpp
void insert_resource(T res) noexcept;
```

Planifie l'insertion d'une ressource dans la scène. Si une ressource de ce type existe déjà, elle sera écrasée.

### remove_resource()

```cpp
void remove_resource<T>() noexcept;
```

Planifie la suppression d'une ressource de la scène.

### despawn()

```cpp
void despawn(Entity e) noexcept;
```

Planifie la suppression d'une entité et de tous ses descendants.

## Exemple d'Utilisation

```cpp
void system(ecs::Commands& commands) {
    // Créer des entités
    ecs::Entity e = commands.spawn(Position{}, Velocity{}).id();

    // Modifier une entité existante
    commands.entity(e).insert(Health{100});

    // Gérer les ressources
    commands.insert_resource(GameState{});

    // Détruire une entité
    commands.despawn(e);
}
```

## Quand les Commandes s'Exécutent

Les commandes sont mises en file d'attente pendant l'exécution des systèmes et sont toutes appliquées à des points de synchronisation spécifiques, généralement après la fin d'un schedule. Cela évite l'invalidation des itérateurs et assure un état du monde cohérent pour tous les systèmes s'exécutant dans la même étape.

```
Le système A s'exécute → commandes mises en file d'attente
Le système B s'exécute → commandes mises en file d'attente
→ Point de synchronisation : Toutes les commandes en attente sont appliquées ici
Le système C s'exécute → voit les changements de A et B
```

## Voir Aussi

- [EntityCommands](./entity-commands.md) - Pour enchaîner les modifications sur une seule entité.
- [Scene](./scene.md) - Le conteneur sous-jacent pour toutes les données ECS.
