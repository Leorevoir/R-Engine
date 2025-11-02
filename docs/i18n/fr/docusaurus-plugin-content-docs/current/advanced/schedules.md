---
sidebar_position: 4
---

# Schedules

Les schedules (planificateurs) organisent l'exécution des systèmes en phases distinctes.

## Schedules Intégrés

R-Engine fournit plusieurs schedules par défaut définis dans l'énumérateur `Schedule` :

- `PRE_STARTUP` : S'exécute une fois avant la phase principale de démarrage. Utile pour mettre en place des ressources fondamentales.
- `STARTUP` : S'exécute une fois au démarrage de l'application, avant la boucle principale. Idéal pour créer les entités initiales et charger les ressources.
- `UPDATE` : Le schedule principal de la boucle de jeu, s'exécute à chaque frame. La plupart de la logique de jeu va ici.
- `FIXED_UPDATE` : S'exécute à un pas de temps fixe, adapté aux calculs de physique.
- `BEFORE_RENDER_3D` / `RENDER_3D` / `AFTER_RENDER_3D` : Phases pour le rendu 3D.
- `BEFORE_RENDER_2D` / `RENDER_2D` / `AFTER_RENDER_2D` : Phases pour le rendu 2D et l'interface utilisateur.
- `EVENT_CLEANUP` : S'exécute à la fin de la frame pour nettoyer les événements.
- `SHUTDOWN` : S'exécute une fois lorsque l'application se ferme.

## Utiliser les Schedules

Vous assignez des systèmes à un schedule lorsque vous les ajoutez à l'application.

```cpp
Application{}
    .add_systems<setup_scene>(Schedule::STARTUP)
    .add_systems<player_input, enemy_ai>(Schedule::UPDATE)
    .add_systems<physics_step>(Schedule::FIXED_UPDATE)
    .add_systems<render_models>(Schedule::RENDER_3D)
    .add_systems<render_ui>(Schedule::RENDER_2D)
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

### Ordre d'Exécution de la Frame

Les schedules s'exécutent dans un ordre prédéfini à chaque frame :

```
(PRE_STARTUP -> STARTUP une fois)
↓
<boucle principale>
  UPDATE
  ↓
  FIXED_UPDATE (zéro ou plusieurs fois)
  ↓
  BEFORE_RENDER_3D -> RENDER_3D -> AFTER_RENDER_3D
  ↓
  BEFORE_RENDER_2D -> RENDER_2D -> AFTER_RENDER_2D
  ↓
  EVENT_CLEANUP
(répéter la boucle principale)
↓
SHUTDOWN
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

app.add_systems<player_movement, enemy_ai>(Schedule::UPDATE)
   .in_set<LogicSet>();

// Ordonner l'exécution de la logique après l'entrée
app.configure_sets<LogicSet>(Schedule::UPDATE)
   .after<InputSet>();

// Vous pouvez aussi spécifier des dépendances directement en ajoutant des systèmes
app.add_systems<collision_detection>(Schedule::UPDATE)
   .in_set<PhysicsSet>()
   .after<LogicSet>(); // Le PhysicsSet s'exécute après le LogicSet
```

## Bonnes Pratiques

### Organiser par Phase

Utilisez le schedule approprié pour chaque type de logique.

```cpp
// Phase d'entrée
app.add_systems<keyboard_input, mouse_input>(Schedule::UPDATE);

// Phase de physique
app.add_systems<collision, physics_solver>(Schedule::FIXED_UPDATE);

// Phase de rendu
app.add_systems<sprite_render, ui_render>(Schedule::RENDER_2D);
```

### Utiliser les Sets pour les Dépendances Générales

Les "system sets" sont excellents pour établir un ordre de haut niveau entre différentes parties de votre application, comme s'assurer que toute la logique de jeu s'exécute avant tous les calculs de physique au sein du même schedule.

```cpp
struct GameLogicSet {};
struct PhysicsSet {};

app.add_systems<player_logic, enemy_logic>(Schedule::UPDATE)
   .in_set<GameLogicSet>();

app.add_systems<collision_system, solver_system>(Schedule::UPDATE)
   .in_set<PhysicsSet>();

// Configurer l'ensemble du PhysicsSet pour qu'il s'exécute après le GameLogicSet
app.configure_sets<PhysicsSet>(Schedule::UPDATE)
   .after<GameLogicSet>();
```

## Exemple : Configuration Complète

```cpp
Application{}
    // Démarrage
    .add_systems<load_assets, spawn_player>(Schedule::STARTUP)

    // Mise à jour : Logique de Jeu et Entrées
    .add_systems<input_system>(Schedule::UPDATE)
    .add_systems<
        player_movement,
        enemy_ai,
        projectile_movement
    >(Schedule::UPDATE)
    .after<input_system>()

    // Mise à jour Fixe : Physique
    .add_systems<collision_detection, physics_solver>(Schedule::FIXED_UPDATE)

    // Rendu
    .add_systems<render_models>(Schedule::RENDER_3D)
    .add_systems<render_ui>(Schedule::RENDER_2D)
    .run();
```

## Prochaines Étapes

- Consultez les [Exemples](../examples/index.md) pour des applications complètes
- Découvrez la [Référence API](../api/index.md) pour les détails techniques
