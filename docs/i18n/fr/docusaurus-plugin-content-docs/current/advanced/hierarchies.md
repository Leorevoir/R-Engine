---
sidebar_position: 2
---

# Hiérarchies

Les hiérarchies permettent des relations parent-enfant entre entités, utiles pour les transformations, dispositions UI et graphes de scène.

## Créer des Hierarchies

### Avec des Enfants

Créer des entités avec des enfants :

```cpp
void setup(Commands& commands) {
    // Créer le parent
    Entity parent = commands.spawn(Transform{}).id();
    
    // Ajouter des enfants
    commands.entity(parent).with_children([&](Commands& child_commands) {
        child_commands.spawn(Transform{});
        child_commands.spawn(Transform{});
    });
}
```

### Définir le Parent

Définir le parent après la création :

```cpp
Entity parent = commands.spawn(Transform{}).id();
Entity child = commands.spawn(Transform{}).id();

commands.entity(child).set_parent(parent);
```

## Parcourir les Hierarchies

### Obtenir les Enfants

```cpp
void system(Query<Entity, Ref<Children>> query) {
    for (auto [entity, children] : query) {
        for (Entity child : children->entities) {
            // Traiter l'enfant
        }
    }
}
```

### Obtenir le Parent

```cpp
void system(Query<Entity, Ref<Parent>> query) {
    for (auto [entity, parent] : query) {
        Entity parent_entity = parent->entity;
        // Traiter le parent
    }
}
```

## Hiérarchie de Transformation

Cas d'usage courant - propagation de transformation :

```cpp
struct Transform {
    Vec3 position;
    Vec3 rotation;
    Vec3 scale = {1, 1, 1};
};

struct GlobalTransform {
    Mat4 matrix;
};

void transform_propagation(
    Query<Mut<GlobalTransform>, Ref<Transform>, Ref<Parent>> query,
    Query<Ref<GlobalTransform>> parent_query
) {
    for (auto [global, local, parent] : query) {
        auto parent_global = parent_query.get<Ref<GlobalTransform>>(parent->entity);
        if (parent_global) {
            global->matrix = parent_global->matrix * local->to_matrix();
        }
    }
}
```

## Retirer de la Hiérarchie

```cpp
// Retirer l'enfant du parent
commands.entity(child).remove_parent();

// Détruire avec les enfants
commands.entity(parent).despawn_recursive();
```

## Bonnes Pratiques

- Utilisez les hiérarchies pour le regroupement logique
- Propagez les transformations du parent vers l'enfant
- Soyez prudent avec les opérations récursives

## Prochaines Étapes

- Apprenez les [Run Conditions](./run-conditions.md)
- Consultez les [Exemples](../examples/system-hierarchy.md)
