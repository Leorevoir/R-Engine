---
sidebar_position: 1
---

# Entités

Les entités sont des identifiants uniques qui représentent des objets dans votre application. Elles sont légères et servent de handle pour regrouper un ensemble de composants.

## Qu'est-ce qu'une Entité ?

Une entité est simplement un ID unique (un entier non signé de 32 bits, alias `ecs::Entity`). Elle n'a ni données ni comportement propre. Son seul but est d'associer une collection de composants.

```cpp
// Une entité est juste un ID
ecs::Entity player_id = 42;
```

## Créer des Entités

Les entités sont créées en utilisant le paramètre système `ecs::Commands`.

```cpp
void spawn_system(ecs::Commands& commands) {
    // Créer une entité vide
    ecs::Entity empty_entity = commands.spawn().id();

    // Créer une entité avec quelques composants initiaux
    ecs::Entity player = commands.spawn(
        Position{0.0f, 0.0f},
        Velocity{1.0f, 0.0f},
        Health{100}
    ).id();
}
```

## Identité de l'Entité

Chaque entité a un identifiant unique qui reste valide jusqu'à ce que l'entité soit détruite.

```cpp
void system(ecs::Commands& commands) {
    ecs::Entity e1 = commands.spawn().id();
    ecs::Entity e2 = commands.spawn().id();

    // e1 ne sera pas égal à e2
}
```

:::caution Cycle de Vie de l'Entité
Une fois qu'une entité est détruite, son ID peut éventuellement être réutilisé par le moteur. Évitez de stocker des ID `Entity` bruts pendant de longues périodes. Si vous avez besoin d'une référence stable, envisagez de créer une ressource qui contient l'ID de l'entité.
:::

## Détruire des Entités

Supprimez les entités et tous leurs composants en utilisant `commands.despawn()`.

```cpp
void cleanup_system(
    ecs::Query<ecs::Entity, ecs::With<Dead>> query,
    ecs::Commands& commands
) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        commands.despawn(it.entity());
    }
}
```

## Relations entre Entités (Hiérarchies)

Les entités peuvent avoir des relations parent-enfant, ce qui est essentiel pour la propagation des transformations et les graphes de scène.

```cpp
void spawn_hierarchy(ecs::Commands& commands) {
    // Créer une entité parente
    commands.spawn(Transform3d{ .position = {400, 300, 0} })
        // Attacher des enfants au parent
        .with_children([](ecs::ChildBuilder& builder) {
            builder.spawn(Transform3d{ .position = {50, 0, 0} });
            builder.spawn(Transform3d{ .position = {-50, 0, 0} });
        });
}
```

[En savoir plus sur les Hiérarchies →](../advanced/hierarchies.md)

## Requêter des Entités

Pour obtenir l'ID d'une entité que vous traitez dans une requête, utilisez la méthode `.entity()` de l'itérateur.

```cpp
void system(ecs::Query<ecs::Ref<Position>> query) {
    for (auto it = query.begin(); it != query.end(); ++it) {
        auto [pos] = *it;
        ecs::Entity entity = it.entity();

        std::cout << "Entité " << entity
                  << " est à (" << pos.ptr->x << ", " << pos.ptr->y << ")\n";
    }
}
```

## Bonnes Pratiques

### ✅ À Faire

- Pensez aux entités comme des handles simples et légers.
- Détruisez les entités lorsqu'elles ne sont plus nécessaires pour libérer des ressources.
- Utilisez des composants marqueurs pour catégoriser les entités (par ex., `struct Player {};`).

````cpp
struct Player {};
struct Enemy {};

// Cela facilite la recherche de types spécifiques d'entités.
void player_system(ecs::Query<ecs::Ref<Position>, ecs::With<Player>> query) {
    // Ce système ne traitera que les entités qui ont un composant Player.
}```

### ❌ À Ne Pas Faire

- Ne stockez pas d'ID d'entités à long terme sans une stratégie pour gérer leur destruction.
- N'essayez pas d'intégrer des données ou de la logique dans l'entité elle-même ; c'est à cela que servent les composants et les systèmes.
- Ne gérez pas les ID d'entités manuellement ; laissez toujours `Commands` gérer la création.

## Prochaines Étapes

- Apprenez-en plus sur les [Composants](./components.md) pour ajouter des données aux entités.
- Voyez comment les [Systèmes](./systems.md) opèrent sur les entités.
- Explorez les [Commandes](./commands.md) pour la manipulation d'entités.
````
