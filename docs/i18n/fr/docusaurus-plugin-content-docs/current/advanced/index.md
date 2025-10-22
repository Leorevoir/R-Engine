---
sidebar_position: 4
---

# Fonctionnalités Avancées

Cette section couvre les fonctionnalités ECS avancées qui vous donnent plus de contrôle et de flexibilité.

## Vue d'Ensemble

Une fois que vous comprenez les concepts fondamentaux, ces fonctionnalités avancées vous aideront à construire des applications plus sophistiquées :

### Events

Communication asynchrone entre systèmes utilisant une architecture orientée événements.

[En savoir plus sur les Events →](./events.md)

### Hierarchies

Relations parent-enfant entre entités pour les graphes de scène et structures imbriquées.

[En savoir plus sur les Hierarchies →](./hierarchies.md)

### Run Conditions

Contrôlez quand les systèmes s'exécutent en fonction de conditions d'exécution.

[En savoir plus sur les Run Conditions →](./run-conditions.md)

### Schedules

Organisez et ordonnez l'exécution des systèmes à travers plusieurs étapes.

[En savoir plus sur les Schedules →](./schedules.md)

## Quand Utiliser les Fonctionnalités Avancées

- **Events** : Quand les systèmes doivent communiquer sans couplage fort
- **Hierarchies** : Pour les transformations, dispositions UI, ou objets de jeu imbriqués
- **Run Conditions** : Pour activer/désactiver les systèmes dynamiquement
- **Schedules** : Pour organiser des dépendances système complexes

## Exemple : Combinaison de Fonctionnalités

```cpp
// Définition d'événement
struct GameStartEvent {};

// Système avec condition d'exécution
bool game_is_running(Res<GameState> state) {
    return state->running;
}

// Configuration de l'application
Application{}
    // Événements
    .add_event<GameStartEvent>()
    
    // Systèmes avec conditions
    .add_systems<game_logic>(Schedule::UPDATE)
    .run_if(run_condition<game_is_running>())
    
    // Configuration hiérarchique
    .add_systems<setup_ui_hierarchy>(Schedule::STARTUP)
    .run();
```

## Prochaines Étapes

Commencez avec les [Events](./events.md) pour apprendre la communication inter-système.
