---
sidebar_position: 3
---

# Conditions d'exécution

Les conditions d'exécution contrôlent quand les systèmes s'exécutent en fonction de l'état d'exécution.

## Que sont les conditions d'exécution ?

Les conditions d'exécution sont des fonctions qui retournent `bool`, déterminant si un système doit s'exécuter.

```cpp
// Définir une condition
bool is_paused(Res<GameState> state) {
    return state->paused;
}

// Appliquer la condition
.add_systems<game_logic>(Schedule::UPDATE)
.run_if(not_run_condition<is_paused>())
```

## Conditions Intégrées

### La Ressource Existe

```cpp
.run_if(resource_exists<Player>())
```

### Ressource Égale

```cpp
bool game_running(Res<GameState> state) {
    return state->mode == GameMode::Playing;
}

.run_if(run_condition<game_running>())
```

## Conditions Personnalisées

```cpp
bool has_enemies(Query<With<Enemy>> query) {
    return !query.is_empty();
}

.add_systems<spawn_wave>(Schedule::UPDATE)
.run_if(not_run_condition<has_enemies>())
```

## Combiner les Conditions

```cpp
.run_if(run_condition<is_playing>())
.run_if(not_run_condition<is_paused>())
```

## Patterns Courants

### Système de Pause

```cpp
struct GameState {
    bool paused = false;
};

bool not_paused(Res<GameState> state) {
    return !state->paused;
}

.add_systems<movement, ai, physics>(Schedule::UPDATE)
.run_if(run_condition<not_paused>())
```

### Mode Debug

```cpp
bool debug_enabled(Res<Config> config) {
    return config->debug;
}

.add_systems<debug_draw, debug_info>(Schedule::UPDATE)
.run_if(run_condition<debug_enabled>())
```

## Performance

Les conditions d'exécution sont évaluées à chaque frame. Gardez-les légères :

```cpp
// ✓ Bon : Vérification simple
bool is_ready(Res<State> state) {
    return state->ready;
}

// ✗ Mauvais : Opération coûteuse
bool has_data(Query<Ref<LargeData>> query) {
    for (auto [data] : query) {
        // Calcul complexe...
    }
    return true;
}
```

## Prochaines Étapes

- Apprenez les [Schedules](./schedules.md)
- Consultez les [Exemples](../examples/index.md)
