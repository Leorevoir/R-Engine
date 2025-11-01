---
sidebar_position: 4
---

# Ressources

Les ressources sont des données globales uniques accessibles par tous les systèmes. Elles sont parfaites pour stocker la configuration, l'état global ou les services.

## Qu'est-ce qu'un Resource?

Une ressource est un élément de données singleton qui existe en dehors des entités. Contrairement aux composants qui sont attachés aux entités, les ressources sont globales et uniques.

```cpp
struct GameConfig {
    float gravity = 9.81f;
    int max_enemies = 100;
    bool debug_mode = false;
};
```

## Ajouter des Resources

### Au Démarrage de l'Application

```cpp
Application{}
    .insert_resource(GameConfig{})
    .insert_resource(DeltaTime{0.016f})
    .run();
```

### Depuis les Systèmes

```cpp
void system(Commands& commands) {
    commands.insert_resource(NewResource{42});
}
```

## Accéder aux Resources

### Accès en Lecture Seule

Utilisez `Res<T>` pour un accès immuable :

```cpp
void physics_system(
    Query<Mut<Velocity>> query,
    Res<GameConfig> config
) {
    for (auto [vel] : query) {
        vel->y += config->gravity;
    }
}
```

### Accès Mutable

Utilisez `ResMut<T>` pour un accès mutable :

```cpp
void score_system(
    ResMut<Score> score,
    EventReader<PointEvent> events
) {
    for (const auto& event : events.iter()) {
        score->value += event.points;
    }
}
```

## Supprimer les Resources

```cpp
void cleanup_system(Commands& commands) {
    commands.remove_resource<TemporaryData>();
}
```

## Common Resource Patterns

### Configuration

```cpp
struct GameSettings {
    float volume = 1.0f;
    bool fullscreen = false;
    int difficulty = 1;
};

// Utilisation dans les systèmes
void apply_settings(Res<GameSettings> settings) {
    set_volume(settings->volume);
    set_fullscreen(settings->fullscreen);
}
```

### Delta Time

```cpp
struct DeltaTime {
    float dt;
    float elapsed;
};

void time_system(ResMut<DeltaTime> time) {
    // Mise à jour à chaque frame
    time->dt = calculate_delta();
    time->elapsed += time->dt;
}
```

### État des Entrées

```cpp
struct Input {
    bool keys[256];
    int mouse_x, mouse_y;
    bool mouse_buttons[3];
};

void input_system(ResMut<Input> input) {
    // Mise à jour de l'état des entrées
    poll_events(input.ptr);
}
```

## Bonnes Pratiques

### ✅ À Faire

- Utilisez les ressources pour les données globales et uniques
- Préférez `Res<T>` à `ResMut<T>` quand c'est possible
- Utilisez les ressources pour les services et la configuration

### ❌ À Éviter

- N'utilisez pas les ressources pour les données spécifiques aux entités (utilisez des composants)
- N'abusez pas des ressources pour tout (utilisez des composants quand approprié)

## Prochaines Étapes

- Apprenez les [Requêtes](./queries.md)
- Explorez les [Commandes](./commands.md)
- Consultez les [Exemples](../examples/index.md)
