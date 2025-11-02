---
sidebar_position: 2
---

# Composants

Les composants sont des structures de données pures qui définissent ce qu'une entité **est** ou **possède**. Ils ne contiennent aucune logique, seulement des données.

## Qu'est-ce qu'un Composant ?

Un composant est une simple `struct` ou `class` qui contient des données. Les composants sont attachés aux entités pour leur donner des propriétés.

```cpp
// Composant de position simple
struct Position {
    float x;
    float y;
};

// Composant de santé
struct Health {
    int current;
    int max;
};

// Composant marqueur (sans données)
struct Player {};
```

## Créer des Composants

### Composants POD (Recommandé)

Utilisez des structures de données simples (Plain Old Data - POD) pour de meilleures performances :

```cpp
struct Velocity {
    float x = 0.0f;
    float y = 0.0f;
};

struct Color {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
};
```

### Composants Complexes

Les composants peuvent contenir des méthodes pour plus de commodité :

```cpp
struct Health {
    float current = 100.0f;
    float max = 100.0f;

    bool is_alive() const {
        return current > 0.0f;
    }

    float percentage() const {
        return current / max;
    }

    void damage(float amount) {
        current = std::max(0.0f, current - amount);
    }
};
```

:::tip Bonne Pratique
Gardez la logique dans les composants au minimum. La plupart de la logique devrait se trouver dans les systèmes.
:::

## Types de Composants

### Composants de Données

Stockent un état ou des propriétés :

```cpp
struct Transform {
    Vec3 position;
    Quaternion rotation;
    Vec3 scale = {1, 1, 1};
};

struct Sprite {
    Texture texture;
    Rect source_rect;
};
```

### Composants Marqueurs

Marquent les entités sans stocker de données :

```cpp
struct Player {};
struct Enemy {};
struct Bullet {};
struct Dead {};

// Utilisation dans une requête
void player_system(Query<Ref<Position>, With<Player>> query) {
    // Traite uniquement les entités avec le composant Player
}
```

### Composants de Drapeaux (Flags)

Stockent des états booléens de manière efficace :

```cpp
struct Flags {
    bool is_visible : 1;
    bool is_solid : 1;
    bool is_animated : 1;
    bool is_hostile : 1;
};
```

## Ajouter des Composants

### À la Création

```cpp
void spawn_system(Commands& commands) {
    // Créer une entité avec plusieurs composants
    commands.spawn(
        Position{100.0f, 200.0f},
        Velocity{0.0f, 0.0f},
        Health{100},
        Player{}
    );
}
```

### Après la Création

```cpp
void system(Query<Entity, Without<Health>> query, Commands& commands) {
    for (auto [entity, _] : query) {
        // Ajouter un composant à une entité existante
        commands.entity(entity).insert(Health{50});
    }
}
```

## Supprimer des Composants

```cpp
void system(Query<Entity, With<Dead>> query, Commands& commands) {
    for (auto [entity, _] : query) {
        // Supprimer un composant spécifique
        commands.entity(entity).remove<AI>();

        // Ou détruire l'entité entière
        commands.entity(entity).despawn();
    }
}
```

## Accéder aux Composants

### Accès Modifiable

Utilisez `Mut<T>` pour modifier les composants :

```cpp
void movement_system(
    Query<Mut<Position>, Ref<Velocity>> query,
    Res<DeltaTime> time
) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x * time->dt;
        pos->y += vel->y * time->dt;
    }
}
```

### Accès en Lecture Seule

Utilisez `Ref<T>` pour un accès en lecture seule :

```cpp
void render_system(Query<Ref<Position>, Ref<Sprite>> query) {
    for (auto [pos, sprite] : query) {
        draw(sprite->texture, pos->x, pos->y);
    }
}
```

## Patrons de Conception de Composants

### Composition plutôt qu'Héritage

Au lieu de hiérarchies d'héritage, composez les entités à partir de composants :

```cpp
// ❌ Mauvais : Héritage profond
class GameObject {};
class Character : public GameObject {};
class Player : public Character {};
class Mage : public Player {};

// ✅ Bon : Composition
struct Player {};
struct Character {};
struct MagicUser {};

// Créer un mage en combinant des composants
commands.spawn(Player{}, Character{}, MagicUser{});
```

### Responsabilité Unique

Gardez les composants focalisés sur une seule préoccupation :

```cpp
// ❌ Mauvais : Composant fourre-tout
struct GameObject {
    Position position;
    Velocity velocity;
    Health health;
    Sprite sprite;
    // Trop de responsabilités !
};

// ✅ Bon : Composants focalisés
struct Position { Vec2 value; };
struct Velocity { Vec2 value; };
struct Health { int value; };
struct Sprite { Texture tex; };
```

### Conception Orientée Données

Stockez les données dans des tableaux (géré par l'ECS) :

```cpp
// L'ECS stocke les composants dans des tableaux contigus :
// Positions: [P1][P2][P3][P4]...
// Velocities: [V1][V2][V3][V4]...
// Très optimisé pour le cache !
```

## Bonnes Pratiques

### ✅ À Faire

- Gardez les composants petits et focalisés
- Utilisez des `struct` POD lorsque c'est possible
- Utilisez des composants marqueurs pour la catégorisation
- Préférez plusieurs petits composants à un seul grand composant

```cpp
// Bon : Composants focalisés
struct Position { Vec2 value; };
struct Velocity { Vec2 value; };
struct CircleCollider { float radius; };
```

### ❌ À Ne Pas Faire

- N'ajoutez pas de logique aux composants (utilisez les systèmes)
- Ne créez pas de composants "dieu" (god components)
- N'utilisez pas l'héritage entre les composants

```cpp
// Mauvais : Trop de logique dans le composant
struct Enemy {
    void update() { /* logique ici */ }
    void draw() { /* rendu ici */ }
    // Devrait être dans des systèmes !
};
```

## Exemple : Configuration Complète d'une Entité

```cpp
// Définir les composants
struct Position { float x, y; };
struct Velocity { float x, y; };
struct Circle { float radius; Color color; };
struct Player {};

// Créer l'entité joueur
void setup(Commands& commands) {
    commands.spawn(
        Position{400.0f, 300.0f},
        Velocity{0.0f, 0.0f},
        Circle{25.0f, Color::Blue},
        Player{}
    );
}

// Les systèmes opèrent sur les composants
void movement(Query<Mut<Position>, Ref<Velocity>> query) {
    for (auto [pos, vel] : query) {
        pos->x += vel->x;
        pos->y += vel->y;
    }
}

void render(Query<Ref<Position>, Ref<Circle>> query) {
    for (auto [pos, circle] : query) {
        draw_circle(pos->x, pos->y, circle->radius, circle->color);
    }
}
```

## Prochaines Étapes

- Apprenez-en plus sur les [Systèmes](./systems.md) pour ajouter de la logique
- Explorez les [Requêtes](./queries.md) pour accéder efficacement aux composants
- Consultez les [Commandes](./commands.md) pour ajouter/supprimer des composants
