---
sidebar_position: 2
---

# Hiérarchies

Les hiérarchies permettent des relations parent-enfant entre entités, utiles pour les transformations, les dispositions d'interface utilisateur et les graphes de scène.

## Créer des Hiérarchies

### Avec des Enfants

La manière principale de créer des hiérarchies est de créer des entités avec des enfants en utilisant le constructeur `with_children` sur `EntityCommands`.

```cpp
void setup(ecs::Commands& commands) {
    // Créer une entité parente
    commands.spawn(Transform3d{})
        // Utiliser with_children pour ajouter des enfants au parent
        .with_children([](ecs::ChildBuilder &builder) {
            // Le constructeur crée des entités qui sont automatiquement rattachées au parent
            builder.spawn(Transform3d{ .position = { 1.0f, 0.0f, 0.0f } });
            builder.spawn(Transform3d{ .position = { -1.0f, 0.0f, 0.0f } });
        });
}
```

Cela ajoute automatiquement un composant `Children` au parent et un composant `Parent` à chaque enfant.

## Parcourir les Hiérarchies

### Obtenir les Enfants

Le composant `Children` contient un vecteur d'entités enfants.

```cpp
void system(ecs::Query<ecs::Ref<ecs::Children>> query) {
    for (auto [children] : query) {
        for (ecs::Entity child : children.ptr->entities) {
            // Traiter l'entité enfant
        }
    }
}
```

### Obtenir le Parent

Le composant `Parent` stocke l'ID de l'entité du parent.

````cpp
void system(ecs::Query<ecs::Ref<ecs::Parent>> query) {
    for (auto [parent] : query) {
        ecs::Entity parent_entity = parent.ptr->entity;
        // Traiter l'entité parente
    }
}```

## Hiérarchie de Transformation

Le cas d'usage le plus courant est la propagation des transformations, que le `TransformPlugin` gère automatiquement.

-   **`Transform3d`** : Représente la position, la rotation et l'échelle locales de l'entité par rapport à son parent.
-   **`GlobalTransform3d`** : Représente la position, la rotation et l'échelle finales de l'entité dans l'espace monde.

Le `transform_propagate_system` s'exécute automatiquement dans le schedule `BEFORE_RENDER_3D`, calculant `GlobalTransform3d` pour toutes les entités en fonction de leur `Transform3d` et du `GlobalTransform3d` de leur parent.

```cpp
// Le moteur fait ça pour vous !
void transform_propagation_concept(
    ecs::Query<ecs::Ref<Transform3d>, ecs::Mut<GlobalTransform3d>, ecs::Ref<ecs::Parent>> query,
    ecs::Query<ecs::Ref<GlobalTransform3d>> parent_query
) {
    for (auto [local, global, parent] : query) {
        if (auto* parent_global = parent_query.get_component_ptr<GlobalTransform3d>(parent.ptr->entity)) {
            // Cette logique est gérée par GlobalTransform3d::from_local_and_parent
            *global.ptr = GlobalTransform3d::from_local_and_parent(*local.ptr, *parent_global);
        }
    }
}
````

## Retirer de la Hiérarchie

Détruire une entité avec `commands.despawn(entity)` détruira également tous ses descendants de manière récursive.

```cpp
// Ceci détruira l'entité parente et tous ses enfants.
commands.despawn(parent_entity);
```

## Bonnes Pratiques

- Utilisez les hiérarchies pour le regroupement logique et la propagation des transformations.
- Modifiez le composant `Transform3d` ; laissez le moteur calculer `GlobalTransform3d`.
- Soyez conscient des opérations récursives lorsque vous parcourez les hiérarchies manuellement.

## Prochaines Étapes

- Apprenez-en plus sur les [Conditions d'Exécution](./run-conditions.md)
- Consultez les [Exemples](../examples/system-hierarchy.md)
