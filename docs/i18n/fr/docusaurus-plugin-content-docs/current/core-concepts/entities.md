---
sidebar_position: 1
---

# Entités

Les entités sont des identifiants uniques qui représentent des objets dans votre application. Elles sont légères et relient les composants entre eux.

## Qu'est-ce qu'une Entité ?

Une entité est simplement un ID unique (généralement un entier 32-bit ou 64-bit). Elle n'a aucune donnée ou comportement propre — c'est juste une référence qui regroupe des composants ensemble.

```cpp
// Une entité est juste un ID
Entity player = Entity{42};
```

## Créer des Entités

Les entités sont créées en utilisant le paramètre système `Commands` :

```cpp
void spawn_system(Commands& commands) {
    // Créer une entité vide
    Entity empty = commands.spawn().id();
    
    // Créer une entité avec des composants
    Entity player = commands.spawn(
        Position{0.0f, 0.0f},
        Velocity{1.0f, 0.0f},
        Health{100}
    ).id();
}
```

## Identité de l'Entité

Chaque entité a un identifiant unique qui reste valide jusqu'à ce que l'entité soit détruite :

```cpp
void system(Commands& commands) {
    Entity e1 = commands.spawn().id();
    Entity e2 = commands.spawn().id();
    
    // e1 != e2 (toujours unique)
}
```

:::caution Cycle de Vie de l'Entité
Une fois qu'une entité est détruite, son ID peut être réutilisé. Ne stockez jamais d'IDs d'entités entre les frames à moins d'être certain que l'entité existe toujours.
:::

## Détruire des Entités

Supprimez des entités en utilisant `despawn` :

```cpp
void cleanup_system(
    Query<Entity, With<Dead>> query,
    Commands& commands
) {
    for (auto [entity, _] : query) {
        commands.entity(entity).despawn();
    }
}
```

## Relations Entre Entités

Les entités peuvent avoir des relations parent-enfant en utilisant le système de hiérarchie :

```cpp
void spawn_hierarchy(Commands& commands) {
    // Créer un parent
    Entity parent = commands.spawn(Transform{}).id();
    
    // Créer des enfants
    commands.entity(parent).with_children([&](Commands& child_builder) {
        child_builder.spawn(Transform{});
        child_builder.spawn(Transform{});
    });
}
```

[En savoir plus sur les Hiérarchies →](../advanced/hierarchies.md)

## Requêter des Entités

Obtenez des entités dans les requêtes en utilisant le type `Entity` :

```cpp
void system(Query<Entity, Ref<Position>> query) {
    for (auto [entity, pos] : query) {
        std::cout << "Entity " << entity.id() 
                  << " at (" << pos->x << ", " << pos->y << ")\n";
    }
}
```

## Bonnes Pratiques

### ✅ À Faire

- Utilisez les entités comme des références légères
- Détruisez les entités quand elles ne sont plus nécessaires
- Utilisez des composants marqueurs pour catégoriser les entités

```cpp
struct Player {};
struct Enemy {};
struct Bullet {};

// Facile de requêter des types d'entités spécifiques
void player_system(Query<Ref<Position>, With<Player>> query) {
    // Traite uniquement les entités joueurs
}
```

### ❌ À Éviter

- Ne stockez pas d'IDs d'entités pendant de longues périodes
- N'utilisez pas les entités comme conteneurs de données (utilisez des composants à la place)
- Ne gérez pas manuellement les IDs d'entités

## Exemple : Pool d'Entités

```cpp
// Créer plusieurs entités
void spawn_enemies(Commands& commands) {
    for (int i = 0; i < 10; i++) {
        commands.spawn(
            Enemy{},
            Position{rand() % 800, rand() % 600},
            Health{50}
        );
    }
}

// Traiter tous les ennemis
void enemy_ai_system(
    Query<Entity, Mut<Position>, Ref<Health>, With<Enemy>> query,
    Commands& commands
) {
    for (auto [entity, pos, health, _] : query) {
        // Logique IA...
        
        // Détruire si mort
        if (health->current <= 0) {
            commands.entity(entity).despawn();
        }
    }
}
```

## Prochaines Étapes

- Découvrez les [Composants](./components.md) pour ajouter des données aux entités
- Voyez comment les [Systèmes](./systems.md) opèrent sur les entités
- Explorez les [Commandes](./commands.md) pour manipuler les entités
