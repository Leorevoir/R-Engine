---
sidebar_position: 4
---

# Scheduldes

Les schedules organisent l'exécution des systèmes en phases distinctes.

## Schedules Intégrés

R-Engine fournit plusieurs schedules par défaut :

```cpp
Schedule::STARTUP    // Exécuté une fois au démarrage de l'application
Schedule::PRE_UPDATE // Avant la mise à jour principale
Schedule::UPDATE     // Boucle de jeu principale
Schedule::POST_UPDATE // Après la mise à jour
Schedule::RENDER_2D  // Rendu 2D
Schedule::RENDER_3D  // Rendu 3D
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

```cpp
.add_systems<input, movement, collision>(Schedule::UPDATE)
.after<input>()  // movement s'exécute après input
```

### Entre les Schedules

Les schedules s'exécutent dans un ordre prédéfini :

```
STARTUP (une fois)
↓
PRE_UPDATE
↓
UPDATE
↓
POST_UPDATE
↓
RENDER_2D
↓
RENDER_3D
↓
(répéter depuis PRE_UPDATE)
```

## System Sets

Groupez les systèmes liés :

```cpp
enum class GameSets {
    Input,
    Logic,
    Physics,
    Rendering
};

.add_systems<input_system>(Schedule::UPDATE)
.in_set(GameSets::Input)

.add_systems<movement, ai>(Schedule::UPDATE)
.in_set(GameSets::Logic)
.after_set(GameSets::Input)

.add_systems<physics>(Schedule::UPDATE)
.in_set(GameSets::Physics)
.after_set(GameSets::Logic)
```

## Bonnes Pratiques

### Organiser par Phase

```cpp
// Phase d'entrée
.add_systems<keyboard_input, mouse_input>(Schedule::PRE_UPDATE)

// Phase de logique
.add_systems<player_movement, enemy_ai>(Schedule::UPDATE)

// Phase de physique
.add_systems<collision, physics>(Schedule::POST_UPDATE)

// Phase de rendu
.add_systems<sprite_render, ui_render>(Schedule::RENDER_2D)
```

### Utiliser les Sets pour les Dépendances

```cpp
.add_systems<game_logic_a, game_logic_b>(Schedule::UPDATE)
.in_set(GameSets::Logic)

.add_systems<render_a, render_b>(Schedule::RENDER_2D)
.in_set(GameSets::Rendering)
.after_set(GameSets::Logic)
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
    .add_systems<collision_detection, physics>(Schedule::POST_UPDATE)
    
    // Rendu
    .add_systems<render_sprites, render_ui>(Schedule::RENDER_2D)
    .run();
```

## Prochaines Étapes

- Consultez les [Exemples](../examples/index.md) pour des applications complètes
- Découvrez la [Référence API](../api/index.md) pour les détails techniques
