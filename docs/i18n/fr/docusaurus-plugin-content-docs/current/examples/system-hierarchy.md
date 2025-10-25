---
sidebar_position: 2
---

# Exemple Hiérarchie de Systèmes

Démontre les relations parent-enfant entre entités utilisant les hiérarchies.

## Vue d'Ensemble

Cet exemple montre comment créer et gérer des structures d'entités hiérarchiques, utile pour :
- Propagation de transformation
- Graphes de scène
- Dispositions UI
- Objets de jeu imbriqués

## Configuration

```cpp
struct Transform {
    Vec2f position;
    float rotation;
    float scale = 1.0f;
};

struct GlobalTransform {
    Vec2f position;
    float rotation;
    float scale = 1.0f;
};
```

## Créer la Hiérarchie

```cpp
void spawn_hierarchy(Commands& commands) {
    // Créer l'entité parent
    Entity parent = commands.spawn(
        Transform{Vec2f{400, 300}, 0.0f, 1.0f},
        GlobalTransform{}
    ).id();
    
    // Ajouter des enfants
    commands.entity(parent).with_children([&](Commands& child_cmds) {
        // Enfant 1 - décalé vers la droite
        child_cmds.spawn(
            Transform{Vec2f{50, 0}, 0.0f, 0.5f},
            GlobalTransform{}
        );
        
        // Enfant 2 - décalé vers la gauche
        child_cmds.spawn(
            Transform{Vec2f{-50, 0}, 0.0f, 0.5f},
            GlobalTransform{}
        );
    });
}
```

## Propagation de Transformation

```cpp
void propagate_transforms(
    Query<Mut<GlobalTransform>, Ref<Transform>, Without<Parent>> root_query,
    Query<Mut<GlobalTransform>, Ref<Transform>, Ref<Parent>> child_query,
    Query<Ref<GlobalTransform>> parent_transform_query
) {
    // Mettre à jour les entités racine (sans parent)
    for (auto [global, local, _] : root_query) {
        global.ptr->position = local.ptr->position;
        global.ptr->rotation = local.ptr->rotation;
        global.ptr->scale = local.ptr->scale;
    }
    
    // Mettre à jour les enfants (propager depuis le parent)
    for (auto [global, local, parent] : child_query) {
        auto parent_global = parent_transform_query
            .get<Ref<GlobalTransform>>(parent.ptr->entity);
        
        if (parent_global) {
            // Combiner les transformations parent et locale
            global.ptr->position = parent_global.ptr->position + 
                                  local.ptr->position * parent_global.ptr->scale;
            global.ptr->rotation = parent_global.ptr->rotation + 
                                  local.ptr->rotation;
            global.ptr->scale = parent_global.ptr->scale * 
                               local.ptr->scale;
        }
    }
}
```

## Système de Rotation

```cpp
void rotate_parent(
    Query<Mut<Transform>, Without<Parent>> query,
    Res<DeltaTime> time
) {
    for (auto [transform, _] : query) {
        transform.ptr->rotation += 1.0f * time.ptr->dt;
    }
}
```

## Rendu

```cpp
void render_hierarchy(Query<Ref<GlobalTransform>> query) {
    for (auto [transform] : query) {
        draw_circle_at(
            transform.ptr->position,
            10.0f * transform.ptr->scale,
            Color::Blue
        );
    }
}
```

## Application

```cpp
int main() {
    Application{}
        .insert_resource(DeltaTime{})
        .add_systems<spawn_hierarchy>(Schedule::STARTUP)
        .add_systems<
            rotate_parent,
            propagate_transforms
        >(Schedule::UPDATE)
        .after<rotate_parent>()  // Propager après la rotation
        .add_systems<render_hierarchy>(Schedule::RENDER_2D)
        .run();
    
    return 0;
}
```

## Concepts Clés

1. **Composants Hiérarchie** : Les composants `Parent` et `Children` relient les entités
2. **Local vs Global** : Transformations séparées pour local et espace-monde
3. **Propagation de Transformation** : Les enfants héritent des transformations parent
4. **Ordre des Systèmes** : La propagation doit s'exécuter après les mises à jour du parent

## Exécution

```bash
./r-engine__system_hierarchy
```

## Prochaines Étapes

- En savoir plus sur les [Hiérarchies](../advanced/hierarchies.md)
- Essayez [Communication par Événements](./event-communication.md)
