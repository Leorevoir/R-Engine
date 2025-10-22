---
sidebar_position: 2
---

# API des commandes

Les commandes fournissent des modifications différées au monde ECS.

## Méthodes

### spawn()

```cpp
EntityCommands spawn()
EntityCommands spawn(Components... components)
```

Met en file d'attente la création d'entité.

**Retourne** : EntityCommands pour configuration supplémentaire

### entity()

```cpp
EntityCommands entity(Entity e)
```

Obtient les commandes pour une entité spécifique.

### insert_resource()

```cpp
void insert_resource(T resource)
```

Met en file d'attente l'insertion de ressource.

### remove_resource()

```cpp
void remove_resource<T>()
```

Met en file d'attente la suppression de ressource.

## Exemple d'Utilisation

```cpp
void system(Commands& commands) {
    // Créer des entités
    Entity e = commands.spawn(Position{}, Velocity{}).id();
    
    // Modifier des entités
    commands.entity(e).insert(Health{100});
    
    // Gérer les ressources
    commands.insert_resource(GameState{});
}
```

## Quand les Commandes s'Exécutent

Les commandes sont appliquées entre les exécutions de systèmes :

```
Le système A s'exécute → commandes en file d'attente
Le système B s'exécute → commandes en file d'attente
→ Commandes appliquées ici
Le système C s'exécute → voit les changements
```

## Voir Aussi

- [EntityCommands](./entity-commands.md) - Commandes par entité
- [Scene](./scene.md) - Accès direct au monde
