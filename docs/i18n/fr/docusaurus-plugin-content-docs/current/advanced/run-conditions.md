---
sidebar_position: 3
---

# Conditions d'exécution

Les conditions d'exécution contrôlent quand les systèmes s'exécutent en fonction de l'état d'exécution.

## Que sont les conditions d'exécution ?

Les conditions d'exécution sont des fonctions prédicats qui retournent `bool`, déterminant si un système (ou un groupe de systèmes) doit s'exécuter dans une frame donnée.

```cpp
// Définir un énumérateur d'état
enum class GameStateEnum { Paused, Running };

// Définir une condition qui vérifie l'état
bool is_running(ecs::Res<State<GameStateEnum>> state) {
    return state.ptr && state.ptr->current() == GameStateEnum::Running;
}

// Appliquer la condition aux systèmes
app.add_systems<player_movement, enemy_ai>(Schedule::UPDATE)
   .run_if<is_running>();
```

## Enchaîner les Conditions

Vous pouvez enchaîner les conditions en utilisant des opérateurs logiques.

- `.run_if<P>()` : Remplace toute condition existante.
- `.run_unless<P>()` : Un `run_if` négatif. S'exécute si le prédicat est faux.
- `.run_and<P>()` : Ajoute une condition qui doit aussi être vraie (ET).
- `.run_or<P>()` : Ajoute une condition qui peut aussi être vraie (OU).

```cpp
.add_systems<special_power_system>(Schedule::UPDATE)
    .run_if<is_running>()
    .run_and<player_has_mana>()
    .run_unless<player_is_stunned>();
```

## Conditions Intégrées

Le moteur fournit plusieurs conditions d'exécution communes dans l'espace de noms `r::run_conditions`.

### `in_state<StateValue>`

Vérifie si une machine à états est dans un état spécifique.

````cpp
// S'exécute seulement quand le jeu est dans l'état 'Playing'.
.run_if<run_conditions::in_state<AppState::Playing>>()```

### `state_changed<T>`

S'exécute pendant un cycle de mise à jour lorsque l'état de type `T` change.

```cpp
// S'exécute seulement sur la frame où AppState change.
.run_if<run_conditions::state_changed<AppState>>()
````

### `on_event<T>`

S'exécute si des événements de type `T` ont été envoyés cette frame.

```cpp
// S'exécute seulement si un PlayerDeathEvent a été déclenché.
.run_if<run_conditions::on_event<PlayerDeathEvent>>()
```

### `resource_exists<T>`

S'exécute si une ressource de type `T` existe.

```cpp
.run_if<run_conditions::resource_exists<SpecialPowerup>>()
```

## Conditions Personnalisées

Toute fonction qui peut être un système peut aussi être une condition d'exécution, tant qu'elle retourne `bool`.

```cpp
bool has_enemies(ecs::Query<ecs::With<Enemy>> query) {
    return query.size() > 0;
}

.add_systems<spawn_wave>(Schedule::UPDATE)
   .run_unless<has_enemies>(); // run_if<...>() fonctionne aussi !
```

## Performance

Les conditions d'exécution sont évaluées à chaque frame avant que leurs systèmes ne s'exécutent. Gardez-les légères.

```cpp
// ✓ Bon : Vérification simple, très rapide.
bool is_ready(ecs::Res<State> state) {
    return state.ptr->ready;
}

// ✗ Mauvais : Itère sur de nombreux composants, potentiellement lent.
bool expensive_check(ecs::Query<ecs::Ref<LargeData>> query) {
    for (auto [data] : query) {
        // Calcul complexe...
    }
    return true;
}
```

## Prochaines Étapes

- Apprenez-en plus sur les [Schedules](./schedules.md)
- Consultez les [Exemples](../examples/index.md)
