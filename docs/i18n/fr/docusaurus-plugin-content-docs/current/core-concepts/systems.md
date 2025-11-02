---
sidebar_position: 3
---

# Systèmes

Les systèmes sont des fonctions qui contiennent la logique de votre application. Ils opèrent sur les entités en interrogeant les composants et peuvent accéder aux ressources globales.

## Qu'est-ce qu'un Système ?

Un système est une fonction C++ ordinaire qui prend des types ECS spéciaux comme paramètres. Le moteur utilise ces paramètres pour fournir les données nécessaires depuis la `Scene`.

```cpp
// Un système qui déplace les entités en fonction de leur vélocité.
void movement_system(
    ecs::Query<ecs::Mut<Position>, ecs::Ref<Velocity>> query,
    ecs::Res<FrameTime> time
) {
    for (auto [pos, vel] : query) {
        pos.ptr->x += vel.ptr->x * time.ptr->delta_time;
        pos.ptr->y += vel.ptr->y * time.ptr->delta_time;
    }
}
```

## Paramètres des Systèmes

Les systèmes peuvent accepter divers types de paramètres qui sont automatiquement injectés par le moteur :

| Type                  | Description                                             | Cas d'utilisation                                           |
| --------------------- | ------------------------------------------------------- | ----------------------------------------------------------- |
| `ecs::Query<...>`     | Itérer sur les entités avec des composants spécifiques. | La manière principale de traiter plusieurs entités.         |
| `ecs::Res<T>`         | Accès en lecture seule à une ressource globale.         | Lire la configuration, le temps, l'état des entrées.        |
| `ecs::ResMut<T>`      | Accès modifiable à une ressource globale.               | Modifier l'état global comme le score ou les paramètres.    |
| `ecs::Commands`       | Un tampon pour les modifications différées du monde.    | Créer/détruire des entités, ajouter/retirer des composants. |
| `ecs::EventWriter<T>` | Un expéditeur pour un type d'événement spécifique.      | Communication asynchrone entre les systèmes.                |
| `ecs::EventReader<T>` | Un récepteur pour un type d'événement spécifique.       | Réagir aux événements envoyés par d'autres systèmes.        |

[En savoir plus sur les Requêtes →](./queries.md)
[En savoir plus sur les Ressources →](./resources.md)
[En savoir plus sur les Commandes →](./commands.md)
[En savoir plus sur les Événements →](../advanced/events.md)

## Enregistrer les Systèmes

Vous ajoutez des systèmes à votre application et les assignez à un `Schedule`.

```cpp
Application{}
    // Les systèmes STARTUP s'exécutent une fois au début.
    .add_systems<setup_game, load_assets>(Schedule::STARTUP)

    // Les systèmes UPDATE s'exécutent à chaque frame.
    .add_systems<player_input, movement, collision>(Schedule::UPDATE)

    // Les systèmes RENDER gèrent le dessin.
    .add_systems<render_sprites>(Schedule::RENDER_2D)
    .run();
```

## Ordre des Systèmes

### Ordre Automatique

Par défaut, le planificateur tente d'exécuter les systèmes en parallèle si leur accès aux données ne crée pas de conflit.

### Dépendances Explicites

Utilisez `.after<>()` et `.before<>()` pour définir un ordre d'exécution spécifique lorsque c'est nécessaire.

```cpp
app.add_systems<movement_system>(Schedule::UPDATE)
   .after<input_system>(); // Assure que movement_system s'exécute après input_system

app.add_systems<cleanup_system>(Schedule::UPDATE)
   .before<render_system>(); // Assure que cleanup s'exécute avant render
```

### Ensembles de Systèmes (System Sets)

Groupez les systèmes liés en ensembles pour gérer leurs dépendances collectivement. Les ensembles sont définis par des `struct` vides.

```cpp
struct PhysicsSet {};
struct RenderingSet {};

app.add_systems<physics_a, physics_b>(Schedule::UPDATE)
   .in_set<PhysicsSet>();

app.add_systems<render_a, render_b>(Schedule::UPDATE)
   .in_set<RenderingSet>();

// Configurer l'ensemble RenderingSet pour qu'il s'exécute après le PhysicsSet
app.configure_sets<RenderingSet>(Schedule::UPDATE)
   .after<PhysicsSet>();
```

[En savoir plus sur les Schedules et les Sets →](../advanced/schedules.md)

## Exécution Conditionnelle

Utilisez des conditions d'exécution pour contrôler quand les systèmes s'exécutent.

```cpp
// Une fonction prédicat qui vérifie une ressource.
bool is_not_paused(ecs::Res<GameState> state) {
    return !state.ptr->paused;
}

// Le système ne s'exécutera que si le prédicat retourne vrai.
app.add_systems<game_logic>(Schedule::UPDATE)
   .run_if<is_not_paused>();
```

[En savoir plus sur les Conditions d'Exécution →](../advanced/run-conditions.md)

## Bonnes Pratiques

### ✅ À Faire

- Gardez les systèmes focalisés sur une seule responsabilité (par ex., un système de mouvement, un système de rendu).
- Utilisez les requêtes pour itérer sur les entités.
- Préférez l'accès en lecture seule (`Ref<T>`, `Res<T>`) autant que possible pour permettre plus de parallélisme.
- Utilisez `Commands` pour tous les changements structurels au monde.

```cpp
// Bon : Un système focalisé avec des dépendances claires.
void apply_gravity(ecs::Query<ecs::Mut<Velocity>> query, ecs::Res<Gravity> gravity) {
    for (auto [vel] : query) {
        vel.ptr->y += gravity.ptr->value;
    }
}
```

### ❌ À Ne Pas Faire

- Ne stockez pas d'état à l'intérieur de la fonction système elle-même (par ex., en utilisant des variables `static`). Utilisez plutôt des ressources ou des composants.
- Ne modifiez pas le monde directement pendant une itération (par ex., en appelant `scene.add_component`). Utilisez `Commands`.
- Ne créez pas de dépendances circulaires entre les systèmes.

```cpp
// Mauvais : Stocker de l'état dans un système.
void bad_system() {
    static int counter = 0;  // ❌ Ce n'est pas thread-safe et c'est une mauvaise pratique.
    counter++;               // Utilisez une ressource à la place : ResMut<MyCounter>.
}
```

## Prochaines Étapes

- Apprenez-en plus sur les [Ressources](./resources.md) pour l'état global.
- Explorez les [Requêtes](./queries.md) pour l'accès aux entités.
- Consultez les [Commandes](./commands.md) pour la modification du monde.
- Découvrez les [Fonctionnalités Avancées](../advanced/index.md) pour plus de capacités système.
