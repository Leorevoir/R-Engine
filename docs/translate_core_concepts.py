#!/usr/bin/env python3
"""Traduction complète des fichiers core-concepts"""
import os

FR_DIR = "/home/matt_nytovo/delivery/R-Type/R-Engine/docs/i18n/fr/docusaurus-plugin-content-docs/current/core-concepts"

# Contenu traduit pour chaque fichier
TRANSLATIONS = {
    "entities.md": """---
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
                  << " at (" << pos->x << ", " << pos->y << ")\\n";
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
""",

    "components.md": """---
sidebar_position: 2
---

# Composants

Les composants sont des structures de données pures qui définissent ce qu'une entité **est** ou **possède**.

## Qu'est-ce qu'un Composant ?

Un composant est une simple structure C++ (ou classe) contenant des données. Les composants n'ont pas de logique — ils stockent uniquement des données.

```cpp
// Composants simples
struct Position {
    float x, y;
};

struct Velocity {
    float x, y;
};

struct Health {
    int current;
    int maximum;
};
```

## Ajouter des Composants

Les composants sont attachés aux entités lors de leur création ou plus tard :

```cpp
void spawn_system(Commands& commands) {
    // Ajouter des composants à la création
    commands.spawn(
        Position{0.0f, 0.0f},
        Velocity{1.0f, 0.5f},
        Health{100, 100}
    );
    
    // Ajouter des composants plus tard
    Entity e = commands.spawn().id();
    commands.entity(e).insert(Position{5.0f, 10.0f});
}
```

## Accéder aux Composants

Accédez aux composants dans les systèmes via des requêtes :

```cpp
void movement_system(
    Query<Mut<Position>, Ref<Velocity>> query
) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x;
        pos->y += vel->y;
    }
}
```

## Types de Composants

### Composants de Données

Composants standards qui contiennent des données :

```cpp
struct Transform {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

struct Sprite {
    std::string texture_path;
    glm::vec4 color;
};
```

### Composants Marqueurs

Composants vides utilisés pour le marquage ou la catégorisation :

```cpp
struct Player {};
struct Enemy {};
struct Projectile {};

// Requête uniquement les joueurs
void player_system(
    Query<Ref<Position>, With<Player>> query
) {
    // Traite uniquement les entités avec le marqueur Player
}
```

### Composants avec Méthodes

Les composants peuvent avoir des méthodes utilitaires :

```cpp
struct Health {
    int current;
    int maximum;
    
    void heal(int amount) {
        current = std::min(current + amount, maximum);
    }
    
    void damage(int amount) {
        current = std::max(current - amount, 0);
    }
    
    bool is_alive() const {
        return current > 0;
    }
};
```

:::tip Gardez les Composants Simples
Préférez la logique simple dans les méthodes. La logique complexe devrait aller dans les systèmes.
:::

## Supprimer des Composants

Supprimez des composants des entités :

```cpp
void system(Commands& commands) {
    Entity e = /* ... */;
    
    // Supprimer un composant unique
    commands.entity(e).remove<Velocity>();
    
    // Supprimer plusieurs composants
    commands.entity(e).remove<Velocity, Health>();
}
```

## Composition de Composants

Combinez des composants pour créer des comportements complexes :

```cpp
// Composants atomiques simples
struct Position { float x, y; };
struct Velocity { float x, y; };
struct Gravity { float force; };
struct Sprite { /* ... */ };
struct Collider { /* ... */ };

// Différentes combinaisons = différents comportements
commands.spawn(Position{}, Velocity{}, Sprite{});           // Objet en mouvement
commands.spawn(Position{}, Velocity{}, Gravity{}, Sprite{}); // Objet affecté par la gravité
commands.spawn(Position{}, Collider{}, Sprite{});            // Objet statique avec collision
```

## Bonnes Pratiques

### ✅ À Faire

- Gardez les composants petits et focalisés
- Utilisez des données simples (POD quand c'est possible)
- Préférez la composition à l'héritage
- Utilisez des composants marqueurs pour la catégorisation

```cpp
// Bon : composants petits et focalisés
struct Position { float x, y; };
struct Velocity { float x, y; };
struct Health { int current, maximum; };
```

### ❌ À Éviter

- Ne mettez pas de logique complexe dans les composants
- Ne créez pas de composants "god" géants
- N'utilisez pas d'héritage de composants

```cpp
// Mauvais : composant god
struct GameObject {
    Position pos;
    Velocity vel;
    Health health;
    Sprite sprite;
    AI ai;
    // Trop de responsabilités !
};
```

## Requête de Composants

Filtrez les entités par présence de composants :

```cpp
// Entités AVEC Position ET Velocity
Query<Ref<Position>, Ref<Velocity>>

// Entités AVEC Position SANS Velocity
Query<Ref<Position>, Without<Velocity>>

// Entités avec le marqueur Enemy
Query<Ref<Position>, With<Enemy>>
```

[En savoir plus sur les Requêtes →](./queries.md)

## Exemple : Système de Santé

```cpp
struct Health {
    int current;
    int maximum;
    
    bool is_alive() const { return current > 0; }
};

struct Dead {};  // Composant marqueur

void health_system(
    Query<Entity, Ref<Health>, Without<Dead>> query,
    Commands& commands
) {
    for (auto [entity, health, _] : query) {
        if (!health->is_alive()) {
            commands.entity(entity).insert(Dead{});
        }
    }
}

void cleanup_dead_system(
    Query<Entity, With<Dead>> query,
    Commands& commands
) {
    for (auto [entity, _] : query) {
        commands.entity(entity).despawn();
    }
}
```

## Prochaines Étapes

- Voyez comment les [Systèmes](./systems.md) opèrent sur les composants
- Découvrez les [Requêtes](./queries.md) pour filtrer les composants
- Explorez les [Commandes](./commands.md) pour modifier les composants
"""
}

def main():
    # Écrire entities.md
    entities_path = os.path.join(FR_DIR, "entities.md")
    with open(entities_path, 'w', encoding='utf-8') as f:
        f.write(TRANSLATIONS["entities.md"])
    print(f"✓ Traduit: entities.md")
    
    # Écrire components.md
    components_path = os.path.join(FR_DIR, "components.md")
    with open(components_path, 'w', encoding='utf-8') as f:
        f.write(TRANSLATIONS["components.md"])
    print(f"✓ Traduit: components.md")
    
    print("\\n2 fichiers traduits - je vais continuer avec les autres dans un prochain script")

if __name__ == "__main__":
    main()
