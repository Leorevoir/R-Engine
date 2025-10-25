---
sidebar_position: 6
---

# Commandes

Les commandes fournissent un accès différé pour modifier le monde ECS. Elles préviennent l'invalidation des itérateurs et permettent des changements structurels sécurisés.

## Que sont les Commands?

Les commandes mettent en file d'attente les modifications à appliquer plus tard, garantissant que les requêtes ne se cassent pas pendant l'itération.

```cpp
void system(Query<Entity> query, Commands& commands) {
    for (auto [entity] : query) {
        // Sécurisé : mis en file d'attente pour plus tard
        commands.entity(entity).despawn();
    }
    // Les commandes sont appliquées après l'itération
}
```

## Types de Commandes

### Créer une Entité

```cpp
void system(Commands& commands) {
    // Entité vide
    Entity e = commands.spawn().id();
    
    // Avec des composants
    commands.spawn(
        Position{0, 0},
        Velocity{1, 0}
    );
}
```

### Commandes d'Entité

```cpp
void system(Entity entity, Commands& commands) {
    auto entity_cmds = commands.entity(entity);
    
    // Ajouter un composant
    entity_cmds.insert(Health{100});
    
    // Supprimer un composant
    entity_cmds.remove<AI>();
    
    // Détruire
    entity_cmds.despawn();
}
```

### Commandes de Ressource

```cpp
void system(Commands& commands) {
    // Insérer une ressource
    commands.insert_resource(Config{});
    
    // Supprimer une ressource
    commands.remove_resource<OldResource>();
}
```

## EntityCommands

Enchaînez les opérations sur une seule entité :

```cpp
commands.spawn()
    .insert(Position{0, 0})
    .insert(Velocity{1, 0})
    .insert(Player{});
```

## Exécution des Commandes

Les commandes sont appliquées entre les exécutions de systèmes :

```
Le système A s'exécute
  → Commandes mises en file d'attente
Le système B s'exécute
  → Commandes mises en file d'attente
Commandes appliquées ← Toutes en même temps
Le système C s'exécute
  → Peut voir les changements
```

## Bonnes Pratiques

### ✅ À Faire

- Utilisez les commandes pour tous les changements structurels
- Enchaînez les commandes d'entité pour plus de clarté
- Laissez les commandes s'appliquer automatiquement

### ❌ À Éviter

- N'essayez pas d'appliquer les commandes manuellement
- N'attendez pas de changements immédiats dans le même système

## Exemple

```cpp
void spawn_enemies(Commands& commands) {
    for (int i = 0; i < 10; i++) {
        commands.spawn(
            Enemy{},
            Position{rand() % 800, rand() % 600},
            Health{50}
        );
    }
}

void cleanup(Query<Entity, With<Dead>> query, Commands& commands) {
    for (auto [entity, _] : query) {
        commands.entity(entity).despawn();
    }
}
```

## Prochaines Étapes

- Consultez les [Fonctionnalités Avancées](../advanced/index.md)
- Explorez les [Exemples](../examples/index.md)
