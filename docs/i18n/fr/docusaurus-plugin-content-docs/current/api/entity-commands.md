---
sidebar_position: 3
---

# API des commandes d'entité

Commandes pour modifier une entité spécifique.

## Méthodes

### insert()

```cpp
EntityCommands& insert(Component component)
```

Ajouter un composant à l'entité.

**Retourne** : Self pour le chaînage

### remove()

```cpp
EntityCommands& remove<T>()
```

Supprimer un composant de l'entité.

**Retourne** : Self pour le chaînage

### despawn()

```cpp
void despawn()
```

Mettre l'entité en file d'attente pour destruction.

### set_parent()

```cpp
EntityCommands& set_parent(Entity parent)
```

Définir l'entité parent (crée une hiérarchie).

### with_children()

```cpp
EntityCommands& with_children(Function fn)
```

Créer des enfants pour cette entité.

### id()

```cpp
Entity id() const
```

Obtenir l'ID de l'entité.

## Exemple d'Utilisation

```cpp
void system(Commands& commands) {
    // Créer et configurer une entité
    commands.spawn()
        .insert(Position{0, 0})
        .insert(Velocity{1, 0})
        .insert(Player{});
    
    // Modifier une entité existante
    Entity e = /* ... */;
    commands.entity(e)
        .insert(Health{100})
        .remove<Frozen>();
    
    // Créer une hiérarchie
    Entity parent = commands.spawn(Transform{}).id();
    commands.entity(parent).with_children([&](Commands& child) {
        child.spawn(Transform{});
    });
}
```

## Chaînage de Méthodes

La plupart des méthodes retournent `EntityCommands&` pour un chaînage pratique :

```cpp
commands.entity(e)
    .insert(A{})
    .insert(B{})
    .remove<C>()
    .set_parent(parent);
```

## Voir Aussi

- [Commands](./commands.md) - Commandes générales
- [Hierarchies](../advanced/hierarchies.md) - Relations parent-enfant
