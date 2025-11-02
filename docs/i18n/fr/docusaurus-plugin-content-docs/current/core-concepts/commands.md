---
sidebar_position: 6
---

# Commandes

Les commandes fournissent un accès différé pour modifier le monde ECS. Elles préviennent l'invalidation des itérateurs et permettent des changements structurels sécurisés pendant l'exécution d'un système.

## Que sont les Commandes ?

`ecs::Commands` est un paramètre de système qui met en file d'attente les modifications (comme la création/suppression d'entités ou l'ajout/retrait de composants). Ces changements sont appliqués tous en même temps à un point sûr entre les exécutions de systèmes, garantissant que les requêtes ne se cassent pas pendant que vous itérez dessus.

```cpp
void system(ecs::Query<ecs::Entity, ecs::With<Enemy>> query, ecs::Commands& commands) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        ecs::Entity entity = it.entity();
        // C'est sûr : la suppression est mise en file d'attente et n'affectera pas l'itération actuelle de la requête.
        commands.despawn(entity);
    }
    // Toutes les commandes en file d'attente sont appliquées après la fin de ce système.
}
```

## Types de Commandes

### Créer des Entités

```cpp
void system(ecs::Commands& commands) {
    // Créer une entité sans composants
    ecs::Entity e = commands.spawn().id();

    // Créer une entité avec un ensemble de composants
    commands.spawn(
        Position{0, 0},
        Velocity{1, 0}
    );
}
```

### Modifier des Entités

Utilisez `commands.entity(entity_id)` pour obtenir un constructeur `EntityCommands`.

```cpp
void system(ecs::Entity entity, ecs::Commands& commands) {
    auto entity_cmds = commands.entity(entity);

    // Ajouter un composant
    entity_cmds.insert(Health{100});

    // Retirer un composant
    entity_cmds.remove<AI>();

    // Détruire l'entité
    commands.despawn(entity);
}
```

### Gérer les Ressources

```cpp
void system(ecs::Commands& commands) {
    // Insérer ou mettre à jour une ressource
    commands.insert_resource(Config{});

    // Retirer une ressource
    commands.remove_resource<OldResource>();
}
```

## EntityCommands

La structure `EntityCommands` fournit un patron de conception (builder pattern) pratique pour enchaîner les opérations sur une seule entité :

```cpp
commands.spawn()
    .insert(Position{0, 0})
    .insert(Velocity{1, 0})
    .insert(Player{});
```

## Exécution des Commandes

Les commandes sont appliquées à des points de synchronisation spécifiques, généralement après que tous les systèmes d'un schedule donné ont été exécutés.

```
Le système A s'exécute
  → Les commandes pour la création et les changements de composants sont mises en file d'attente
Le système B s'exécute
  → D'autres commandes sont mises en file d'attente
Point de synchronisation (par ex., fin du schedule UPDATE)
  → Toutes les commandes en file d'attente sont appliquées en une seule fois
Le système C s'exécute
  → Peut voir les changements effectués par les systèmes A et B
```

## Bonnes Pratiques

### ✅ À Faire

- Utilisez `Commands` pour **tous** les changements structurels (création, suppression, ajout/retrait de composants).
- Enchaînez les appels `EntityCommands` pour une meilleure lisibilité.
- Fiez-vous au moteur pour appliquer les commandes automatiquement au bon moment.

### ❌ À Ne Pas Faire

- N'essayez pas d'appliquer les commandes manuellement.
- N'attendez pas que les changements effectués via `Commands` soient visibles au sein du même système qui les a émis.

## Exemple

```cpp
void spawn_enemies(ecs::Commands& commands) {
    for (int i = 0; i < 10; i++) {
        commands.spawn(
            Enemy{},
            Position{rand() % 800, rand() % 600},
            Health{50}
        );
    }
}

void cleanup_system(ecs::Query<ecs::Entity, ecs::With<Dead>> query, ecs::Commands& commands) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        commands.despawn(it.entity());
    }
}
```

## Prochaines Étapes

- Consultez les [Fonctionnalités Avancées](../advanced/index.md)
- Explorez les [Exemples](../examples/index.md)
