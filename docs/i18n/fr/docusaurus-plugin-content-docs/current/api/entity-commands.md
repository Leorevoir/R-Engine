---
sidebar_position: 3
---

# API des EntityCommands

`ecs::EntityCommands` fournit un patron de conception (builder pattern) pour modifier une entité spécifique.

## Méthodes

### insert()

```cpp
EntityCommands& insert(T component) noexcept;
```

Planifie l'ajout d'un composant à l'entité.

**Retourne** : Self pour le chaînage.

### remove()

```cpp
EntityCommands& remove<T>() noexcept;
```

Planifie la suppression d'un composant de l'entité.

**Retourne** : Self pour le chaînage.

### with_children()

```cpp
EntityCommands& with_children(FuncT&& func) noexcept;
```

Crée des entités enfants pour cette entité. La fonction fournie reçoit un `ChildBuilder`.

**Retourne** : Self pour le chaînage.

### id()

```cpp
Entity id() const noexcept;
```

Retourne l'ID de l'entité. Si l'entité vient d'être créée, il s'agit d'un ID de substitution temporaire valide uniquement dans le cycle actuel du tampon de commandes.

## Exemple d'Utilisation

```cpp
void system(ecs::Commands& commands) {
    // Créer et configurer une nouvelle entité
    commands.spawn()
        .insert(Position{0, 0})
        .insert(Velocity{1, 0})
        .insert(Player{});

    // Modifier une entité existante
    ecs::Entity e = /* ... */;
    commands.entity(e)
        .insert(Health{100})
        .remove<Frozen>();

    // Créer une hiérarchie
    commands.spawn(Transform3d{})
        .with_children([](ecs::ChildBuilder& builder) {
            builder.spawn(Transform3d{ .position = {1, 0, 0} });
            builder.spawn(Transform3d{ .position = {-1, 0, 0} });
        });
}
```

## Chaînage de Méthodes

La plupart des méthodes retournent `EntityCommands&` pour un chaînage pratique :

```cpp
commands.entity(e)
    .insert(ComponentA{})
    .insert(ComponentB{})
    .remove<ComponentC>();
```

## Voir Aussi

- [Commands](./commands.md) - L'interface principale pour toutes les modifications du monde.
- [Hierarchies](../advanced/hierarchies.md) - Plus d'informations sur les relations parent-enfant.
