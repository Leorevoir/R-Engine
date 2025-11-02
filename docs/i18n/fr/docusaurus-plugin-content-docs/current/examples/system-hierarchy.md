---
sidebar_position: 2
---

# Exemple de Hiérarchie de Systèmes

Démontre les relations parent-enfant entre entités en utilisant les hiérarchies pour la propagation des transformations.

## Vue d'Ensemble

Cet exemple montre comment créer et gérer des structures d'entités hiérarchiques. Une entité centrale "parente" tourne, et ses entités "enfants" orbitent autour d'elle, héritant de sa transformation. C'est utile pour :

- La propagation des transformations (comme une planète orbitant autour d'un soleil)
- Les graphes de scène
- Les dispositions d'interface utilisateur
- Les objets de jeu imbriqués

## Composants

```cpp
// Transformation locale par rapport au parent
struct Transform3d {
    Vec3f position;
    Vec3f rotation; // Angles d'Euler en radians
    Vec3f scale = {1.0f, 1.0f, 1.0f};
};

// Transformation globale dans l'espace monde, calculée par le moteur
struct GlobalTransform3d : public Transform3d {};
```

## Créer la Hiérarchie

Nous créons une entité parente et y attachons des enfants en utilisant `with_children`.

```cpp
void spawn_hierarchy(ecs::Commands& commands) {
    // Créer l'entité parente (le "soleil")
    commands.spawn(
        Transform3d{ .position = {400, 300, 0} },
        Circle{ .radius = 20.0f, .color = Color::Yellow }
    )
    .with_children([](ecs::ChildBuilder& builder) {
        // Enfant 1 (une "planète")
        builder.spawn(
            Transform3d{ .position = {100, 0, 0}, .scale = {0.5f, 0.5f, 0.5f} },
            Circle{ .radius = 15.0f, .color = Color::Blue }
        );

        // Enfant 2 (une autre "planète")
        builder.spawn(
            Transform3d{ .position = {-150, 0, 0}, .scale = {0.3f, 0.3f, 0.3f} },
            Circle{ .radius = 12.0f, .color = Color::Green }
        );
    });
}
```

## Système de Propagation des Transformations

Le `TransformPlugin` du moteur (inclus dans `DefaultPlugins`) gère automatiquement la propagation des transformations. Il exécute un système qui calcule le `GlobalTransform3d` de chaque entité en fonction de son `Transform3d` local et du `GlobalTransform3d` de son parent. Cela se produit à chaque frame avant le rendu.

## Système de Rotation

Ce système trouve l'entité racine (celle sans composant `Parent`) et fait tourner sa transformation locale. Le système de propagation des transformations s'assurera ensuite que cette rotation est héritée par les enfants.

```cpp
void rotate_parent(
    ecs::Query<ecs::Mut<Transform3d>, ecs::Without<ecs::Parent>> query,
    ecs::Res<core::FrameTime> time
) {
    for (auto [transform, _] : query) {
        // Rotation autour de l'axe Z
        transform.ptr->rotation.z += 1.0f * time.ptr->delta_time;
    }
}
```

## Système de Rendu

Ce système dessine simplement un cercle pour chaque entité à sa position `GlobalTransform3d` finale.

```cpp
void render_hierarchy(ecs::Query<ecs::Ref<GlobalTransform3d>, ecs::Ref<Circle>> query) {
    for (auto [transform, circle] : query) {
        DrawCircle(
            transform.ptr->position.x,
            transform.ptr->position.y,
            circle.ptr->radius,
            circle.ptr->color
        );
    }
}
```

## Configuration de l'Application

```cpp
int main() {
    Application{}
        .add_plugins(DefaultPlugins{}) // Inclut l'essentiel TransformPlugin
        .add_systems<spawn_hierarchy>(Schedule::STARTUP)
        .add_systems<rotate_parent>(Schedule::UPDATE)
        .add_systems<render_hierarchy>(Schedule::RENDER_2D)
        .run();

    return 0;
}
```

## Concepts Clés

1.  **Composants de Hiérarchie** : Les composants `ecs::Parent` et `ecs::Children` lient les entités.
2.  **Local vs Global** : `Transform3d` est pour les modifications locales, `GlobalTransform3d` est le résultat calculé.
3.  **Propagation Automatique** : Le moteur gère les calculs pour s'assurer que les enfants héritent des transformations des parents.
4.  **Ordre des Systèmes** : Le `TransformPlugin` garantit que la propagation a lieu après les mises à jour mais avant le rendu.

## Prochaines Étapes

- Apprenez-en plus sur les [Hiérarchies](../advanced/hierarchies.md)
- Essayez la [Communication par Événements](./event-communication.md)
