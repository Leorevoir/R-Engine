---
sidebar_position: 4
---

# Fonctionnalités Avancées

Cette section couvre les fonctionnalités ECS avancées qui vous donnent plus de contrôle et de flexibilité.

## Vue d'Ensemble

Une fois que vous comprenez les concepts fondamentaux, ces fonctionnalités avancées vous aideront à construire des applications plus sophistiquées :

### Événements

Communication asynchrone entre systèmes utilisant une architecture orientée événements.

[En savoir plus sur les Événements →](./events.md)

### Hiérarchies

Relations parent-enfant entre entités pour les graphes de scène et les structures imbriquées.

[En savoir plus sur les Hiérarchies →](./hierarchies.md)

### Conditions d'Exécution

Contrôlez quand les systèmes s'exécutent en fonction de conditions d'exécution.

[En savoir plus sur les Conditions d'Exécution →](./run-conditions.md)

### Schedules

Organisez et ordonnez l'exécution des systèmes à travers plusieurs étapes.

[En savoir plus sur les Schedules →](./schedules.md)

## Quand Utiliser les Fonctionnalités Avancées

- **Événements** : Quand les systèmes doivent communiquer sans couplage fort
- **Hiérarchies** : Pour les transformations, les dispositions d'interface utilisateur ou les objets de jeu imbriqués
- **Conditions d'Exécution** : Pour activer/désactiver les systèmes dynamiquement
- **Schedules** : Pour organiser des dépendances de systèmes complexes

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

Commencez avec les [Événements](./events.md) pour apprendre la communication inter-système.
