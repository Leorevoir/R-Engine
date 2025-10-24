---
sidebar_position: 4
---

# Schedules

Les schedules (ou planificateurs) organisent l'exécution des systèmes en phases distinctes.

## Schedules Intégrés

R-Engine fournit plusieurs schedules par défaut :

```cpp
Schedule::STARTUP    // Exécuté une fois au démarrage de l'application
Schedule::UPDATE     // Boucle de jeu principale (chaque frame)
Schedule::RENDER_2D  // Rendu 2D
Schedule::RENDER_3D  // Rendu 3D
// ... et plus
```

## Utiliser les Schedules

```cpp
Application{}
    .add_systems<setup>(Schedule::STARTUP)
    .add_systems<input, movement>(Schedule::UPDATE)
    .add_systems<render>(Schedule::RENDER_2D)
    .run();
```

## Ordre des Systèmes

### Au Sein d'un Schedule

Utilisez `.after<System>()` et `.before<System>()` pour définir des dépendances explicites.

```cpp
// Garantit que movement_system s'exécute après input_system dans la même frame
app.add_systems<movement_system>(Schedule::UPDATE)
   .after<input_system>();
```

### Entre les Schedules

Les schedules s'exécutent dans un ordre prédéfini à chaque frame :

```
(STARTUP une fois)
↓
PRE_UPDATE
↓
UPDATE
↓
POST_UPDATE
↓
RENDER_3D
↓
RENDER_2D
↓
(répéter depuis PRE_UPDATE)
```

## System Sets

Groupez les systèmes liés pour gérer leurs dépendances collectivement. Un "set" est défini à l'aide d'une simple `struct` vide.

```cpp
// Définir les sets comme des types simples
struct InputSet {};
struct LogicSet {};
struct PhysicsSet {};

// Ajouter les systèmes aux sets
app.add_systems<keyboard_input, mouse_input>(Schedule::UPDATE)
   .in_set<InputSet>();

// Ordonner les sets les uns par rapport aux autres
app.add_systems<player_movement, enemy_ai>(Schedule::UPDATE)
   .in_set<LogicSet>()
   .after<InputSet>();

app.add_systems<collision_detection>(Schedule::UPDATE)
   .in_set<PhysicsSet>()
   .after<LogicSet>();
```

## Bonnes Pratiques

### Organiser par Phase

```cpp
// Phase d'entrée
app.add_systems<keyboard_input, mouse_input>(Schedule::PRE_UPDATE);

// Phase de logique
app.add_systems<player_movement, enemy_ai>(Schedule::UPDATE);

// Phase de physique
app.add_systems<collision, physics_solver>(Schedule::POST_UPDATE);

// Phase de rendu
app.add_systems<sprite_render, ui_render>(Schedule::RENDER_2D);
```

### Utiliser les Sets pour les Dépendances

```cpp
struct LogicSet {};
struct RenderingSet {};

app.add_systems<game_logic_a, game_logic_b>(Schedule::UPDATE)
   .in_set<LogicSet>();

app.add_systems<render_a, render_b>(Schedule::RENDER_2D)
   .in_set<RenderingSet>();

// Pour s'assurer que le rendu a lieu après la logique, vous pouvez configurer les sets
app.configure_sets<RenderingSet>(Schedule::UPDATE)
   .after<LogicSet>();
```

## Exemple : Configuration Complète

```cpp
Application{}
    // Démarrage
    .add_systems<load_assets, spawn_player>(Schedule::STARTUP)

    // Pré-mise à jour : Entrées
    .add_systems<input_system>(Schedule::PRE_UPDATE)

    // Mise à jour : Logique de Jeu
    .add_systems<
        player_movement,
        enemy_ai,
        projectile_movement
    >(Schedule::UPDATE)
    .after<input_system>()

    // Post-mise à jour : Physique
    .add_systems<collision_detection, physics_solver>(Schedule::POST_UPDATE)

    // Rendu
    .add_systems<render_sprites, render_ui>(Schedule::RENDER_2D)
    .run();
```

## Prochaines Étapes

- Consultez les [Exemples](../examples/index.md) pour des applications complètes
- Découvrez la [Référence API](../api/index.md) pour les détails techniques
